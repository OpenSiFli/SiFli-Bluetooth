/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rtthread.h"
#include "bts2_app_inc.h"

#ifdef CFG_AV_SNK


#ifdef CFG_AV_AAC
    #include <neaacdec.h>
#endif

#ifdef RT_USING_BT
    #include "bt_rt_device.h"
#endif
#include "bt_connection_manager.h"
#include "log.h"
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
    #include "audio_server.h"
    #include "sifli_resample.h"
#endif
#if PKG_USING_VBE_DRC
    #include "vbe_eq_drc_api.h"
    #define A2DP_VBE_OUT_BUFFER_SIZE     8192
#endif

uint8_t   bts2s_avsnk_openFlag;//0x00:dont open a2dp profile; 0x01:open a2dp profile;
//!Used to record the number of sbc frames for each connection
uint8_t   frms_per_payload[MAX_CONNS];

extern bts2_app_stru *bts2g_app_p;

#ifdef A2DP_RELAY_SERVICE
extern U16 a2dp_relay_transfer(U8 *data, U16 len, uint8_t left_cnt, uint32_t left_byte);
extern void a2dp_relay_trigger_audio_server(void);
extern void a2dp_relay_start(U8 codec, BTS2E_SBC_CHNL_MODE chnl_mode, BTS2E_SBC_ALLOC_METHOD alloc_method, U16 sample_freq,
                             U8 nrof_blocks, U8 nrof_subbands, U8 bitpool, U8 start_playing_cnt, U8 max_playing_cnt, U8 target_channel);
extern void a2dp_relay_stop(void);
extern uint8_t a2dp_relay_stereo_enable(void);
extern uint8_t a2dp_relay_get_channel(void);
#endif // A2DP_RELAY_SERVICE


//#include "windows.h"


#define BEGIN_ACCESS_BUF(i)              rt_sem_take(bt_av_get_inst_data()->con[i].snk_data.buf_sem, RT_WAITING_FOREVER)
#define END_ACCESS_BUF(i)                rt_sem_release(bt_av_get_inst_data()->con[i].snk_data.buf_sem)





#ifdef CFG_AV_AAC
static NeAACDecHandle hAac[MAX_NUM_LOCAL_SNK_AAC_SEIDS] = {NULL};
static unsigned long  aac_samplerate[MAX_NUM_LOCAL_SNK_AAC_SEIDS] = {0};
static uint8_t        is_aac_inited[MAX_NUM_LOCAL_SNK_AAC_SEIDS] = {0};
#endif


#define  BIT_RATE_DEAFLUT                  (324)
#define  SINK_DATA_LIST_START_THRESHOLD    (5)
#define  SINK_DATA_LIST_MAX_THRESHOLD      (10)





#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
static sifli_resample_t *resample;
#if BT_BAP_BROADCAST_SOURCE
    static int16_t *g_left;
    static int16_t *g_right;
    static uint32_t g_remain;
    static uint32_t g_offset;
    static uint32_t g_one_channle_size;
    static uint16_t g_drop_cnt = 12;
#endif

static rt_event_t g_playback_evt;
static rt_thread_t g_playback_thread = NULL;
#define  PLAYBACK_GETDATA_EVENT_FLAG       (1 << 0)
#define  PLAYBACK_START_EVENT_FLAG         (1 << 1)
#define  PLAYBACK_STOPPING_EVENT_FLAG      (1 << 2)
#define  PLAYBACK_STOPPED_EVENT_FLAG       (1 << 3)
//!For multiple decoders to work together
#define  PLAYBACK_START_EVENT_FLAG_EXT     (1 << 4)
#define  PLAYBACK_GETDATA_EVENT_FLAG_EXT   (1 << 5)
#define  PLAYBACK_STOPPING_EVENT_FLAG_EXT  (1 << 6)


static uint8_t list_push_back(play_list_t *list, list_hdr_t *hdr, U8 conn_idx)
{
    uint8_t ret;

    RT_ASSERT(hdr != NULL);

    BEGIN_ACCESS_BUF(conn_idx);

    if (list->first == NULL)
    {
        list->first = hdr;
    }
    else
    {
        list->last->next = hdr;
    }
    list->last = hdr;
    hdr->next = NULL;
    list->cnt++;
    ret = (list->cnt >= list->cnt_th) ? 1 : 0;

    //USER_TRACE("(d%d)list->cnt= %d\n", conn_idx, list->cnt);

    //RT_ASSERT(list->cnt <= 100);
    if (list->cnt > SINK_DATA_LIST_MAX_THRESHOLD)
    {
        list->full_num++;
        ret = 2;
    }
    list->total_num++;
    END_ACCESS_BUF(conn_idx);

    return ret;
}

static list_hdr_t *list_pop_front(play_list_t *list, U8 con_idx)
{
    list_hdr_t *hdr;

    BEGIN_ACCESS_BUF(con_idx);

    hdr = list->first;
    if (hdr != NULL)
    {
        list->first = list->first->next;
        if (list->first == NULL)
        {
            list->last = NULL;
        }

        list->cnt--;
    }
    else
    {
        list->empty_num++;
    }

    //USER_TRACE("(d%d)list->cnt= %d\n", con_idx, list->cnt);

    END_ACCESS_BUF(con_idx);

    return hdr;
}

static void list_all_free(play_list_t *list, U8 con_idx)
{
    list_hdr_t *hdr;

    USER_TRACE("(d%d)all free:%x,cnt:%d\n", con_idx, list->first, list->cnt);

    while (list->first)
    {
        hdr = list_pop_front(list, con_idx);
        bfree(hdr);
    }
    list->empty_num = 0;
    list->full_num = 0;
    list->total_num = 0;

}


static int audio_bt_music_client_cb(audio_server_callback_cmt_t cmd, void *userdata, uint32_t unused)
{
    U8 *con_idx = (U8 *)userdata;
    (void)unused;
    if (cmd == as_callback_cmd_cache_empty || cmd == as_callback_cmd_cache_half_empty)
    {
        if (*con_idx == 0)
        {
            rt_event_send(g_playback_evt, PLAYBACK_GETDATA_EVENT_FLAG);
        }
        else
        {
            rt_event_send(g_playback_evt, PLAYBACK_GETDATA_EVENT_FLAG_EXT);
        }
    }

    return 0;
}

static U8 *play_data_decode_data_process(bts2s_av_inst_data *inst, U8 *decode_buf, U16 *out_len)
{
#ifdef A2DP_RELAY_SERVICE
    // stereo to mono
    if (a2dp_relay_stereo_enable())
    {
        uint8_t channel = a2dp_relay_get_channel();
        int16_t *p = (int16_t *)decode_buf;
        uint32_t samples = *out_len >> 2;
        for (uint32_t i = 0; i < samples; i++)
        {
            p[i] = p[i * 2 + channel];
        }
        *out_len = *out_len >> 1;
    }
#endif // A2DP_RELAY_SERVICE
    return decode_buf;
}

static U8 *play_data_decode(bts2s_av_inst_data *inst, U16 *out_len, U8 con_idx)
{
    U8 *frm_ptr, *data, *media_pkt_ptr;
    U16 bytes_left;
    BTS2S_SBC_STREAM bss;
    U8 *ret = NULL;
    play_data_t *pt_data;

    pt_data = (play_data_t *)list_pop_front(&(inst->con[con_idx].snk_data.playlist), con_idx);
    inst->con[con_idx].snk_data.pt_curdata = pt_data;

    *out_len = 0;
    data = (U8 *)pt_data;

    if (pt_data == NULL)
    {
        return ret;
    }

    if (pt_data->len <= (AV_FIXED_MEDIA_PKT_HDR_SIZE + 1))
    {
        USER_TRACE("a2dp play decode len:%d\n", pt_data->len);
        bfree(pt_data);
        return ret;
    }

    if (inst->con[con_idx].snk_data.codec == AV_SBC)
    {
        frm_ptr = data + AV_FIXED_MEDIA_PKT_HDR_SIZE + 1;
        media_pkt_ptr = data + AV_FIXED_MEDIA_PKT_HDR_SIZE;
        bytes_left = pt_data->len - AV_FIXED_MEDIA_PKT_HDR_SIZE - 1;
        bts2_sbc_cfg *cfg = &inst->con[con_idx].act_cfg;

        if (((*media_pkt_ptr) & 0x0f) > frms_per_payload[con_idx])
        {
            frms_per_payload[con_idx] = (*media_pkt_ptr) & 0x0f;

            if (inst->con[con_idx].snk_data.decode_buf)
            {
                bfree(inst->con[con_idx].snk_data.decode_buf);
                inst->con[con_idx].snk_data.decode_buf = NULL;
            }

            if (inst->con[con_idx].snk_data.decode_buf == NULL)
            {
                U16 decode_buffer_size;
                U16 pcm_samples_per_sbc_frame;

                pcm_samples_per_sbc_frame = bts2_sbc_calculate_pcm_samples_per_sbc_frame(cfg->blocks, cfg->subbands);
                decode_buffer_size = pcm_samples_per_sbc_frame * frms_per_payload[con_idx] * 2;
                USER_TRACE("(d%d)frms_per_payload = %d,pcm_samples_per_sbc_frame = %d,decode_buffer_size = %d\n",
                           con_idx, frms_per_payload[con_idx], pcm_samples_per_sbc_frame, decode_buffer_size);
                inst->con[con_idx].snk_data.decode_buf = bmalloc(decode_buffer_size);
                inst->con[con_idx].snk_data.decode_buf_len = decode_buffer_size;
            }
            BT_OOM_ASSERT(inst->con[con_idx].snk_data.decode_buf);
        }

        bss.dst_len = inst->con[con_idx].snk_data.decode_buf_len;
        bss.pdst = inst->con[con_idx].snk_data.decode_buf;
        bss.src_len = bytes_left;
        bss.psrc = frm_ptr;

        bts2_sbc_decode_ext(&bss, con_idx);

        ret = inst->con[con_idx].snk_data.decode_buf;
        *out_len = bss.dst_len_used;
        RT_ASSERT(bss.src_len_used == bss.src_len);
    }
    else if (inst->con[con_idx].snk_data.codec == AV_MPEG24_AAC)
    {
#ifdef CFG_AV_AAC
        U32 t1 = rt_system_get_time();
        U32 t2;
        frm_ptr = data + AV_FIXED_MEDIA_PKT_HDR_SIZE;
        bytes_left = pt_data->len - AV_FIXED_MEDIA_PKT_HDR_SIZE;

        const unsigned char *fin = frm_ptr;
        int fileread = bytes_left;
        char *faad_id_string = NULL;
        char *faad_copyright_string = NULL;
        NeAACDecFrameInfo frameInfo;
        NeAACDecConfigurationPtr config;
        unsigned char channels;
        void *sample_buffer;

#if 0
        if (0 == NeAACDecGetVersion(&faad_id_string, &faad_copyright_string))
        {
        }
        unsigned long cap = NeAACDecGetCapabilities();
#endif
        // Check if decoder has the needed capabilities
        // Open the library
        if (!is_aac_inited[con_idx])
        {
            char err = NeAACDecInit(hAac[con_idx], (unsigned char *)fin, fileread, &aac_samplerate[con_idx],
                                    &channels);
            if (err != 0)
            {
                RT_ASSERT(0);
            }
            is_aac_inited[con_idx] = 1;
        }
        unsigned long frame_index = 0;
        frameInfo.bytesconsumed = 0;


        // Only one frame
        sample_buffer = NeAACDecDecode(hAac[con_idx], &frameInfo, (unsigned char *)fin,  fileread);

        fileread -= frameInfo.bytesconsumed;

        if (frameInfo.error != 0)
        {
            USER_TRACE("decode error %d\n", frameInfo.error);
        }

        if (fileread != 0) // < 2048 is safe if, not a 100% frame data at tail
        {
            USER_TRACE("size not enough %d\n", fileread);
        }


        if (frameInfo.error == 0 && fileread == 0)
        {
            ret = sample_buffer;
            *out_len = 2 * frameInfo.samples;
        }


        t2 = rt_system_get_time();
#endif
        //USER_TRACE("ex t %d(%d, %d)\n", t2 - t1, t2, t1);
    }
    else
    {
        RT_ASSERT(0);
    }


    ret = play_data_decode_data_process(inst, ret, out_len);
    bfree(pt_data);

    return ret;
}

#if BT_BAP_BROADCAST_SOURCE
extern struct rt_ringbuffer *ble_bap_src_enabled_ring;
#define SPEAKER_DMA_SIZE    960
static const uint8_t zero[160] = {0};
void prepare_ble_src_data(int16_t *data, uint32_t len)
{
    int16_t *left, *right, *src;
    uint32_t bytes = sifli_resample_process(resample, data, len, 0); //two channel data bytes
    src = sifli_resample_get_output(resample);
    RT_ASSERT((bytes & 3) == 0);
    uint32_t one_channel_sample = bytes >> 2;
    if ((one_channel_sample * 2 + g_remain) > g_one_channle_size)
    {
        USER_TRACE("sample=%d remain=%d size=%d", one_channel_sample, g_remain, g_one_channle_size);
        RT_ASSERT(0);
    }
    left = g_left + (g_remain >> 1);
    right = g_right + (g_remain >> 1);

    while (one_channel_sample > 0)
    {
        *left++ = *src++;
        *right++ = *src++;
        one_channel_sample--;
    }

    //one channel data_len
    g_remain = (bytes >> 1) + g_remain;
    g_offset = 0;
}

#if defined (RT_USING_FINSH)
int src_drop(int argc, char **argv)
{
    if (argc == 1)
    {
        LOG_I("drop=%d", g_drop_cnt);
    }
    else
    {
        g_drop_cnt = atoi(argv[1]);
        LOG_I("drop=%d", g_drop_cnt);
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(src_drop, src_drop, src_drop);
#endif

void notify_dma_done_to_a2dp()
{
    rt_event_send(g_playback_evt, PLAYBACK_GETDATA_EVENT_FLAG);
}
static void decode_playback_thread(void *args)
{
    bts2s_av_inst_data *inst_data;

    rt_uint32_t evt;
    play_data_t *pt_data;
    U8 *decode_data = NULL;
    U16 decode_len = 0;
    U16 need_send_len = 0;
    U8  is_stopped = 1;
    U8  debug_tx_cnt = 0;
    int  ret_write = 0;
#if PKG_USING_VBE_DRC
    uint32_t vbe_out_size;
#endif
    g_playback_evt = rt_event_create("playback_evt", RT_IPC_FLAG_FIFO);

    while (1)
    {
        evt = 0;
        rt_err_t err = rt_event_recv(g_playback_evt, PLAYBACK_GETDATA_EVENT_FLAG | PLAYBACK_START_EVENT_FLAG | PLAYBACK_STOPPING_EVENT_FLAG, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &evt);
        inst_data = bt_av_get_inst_data();
        if (evt & PLAYBACK_STOPPING_EVENT_FLAG)
        {
            is_stopped = 1;
            rt_event_send(g_playback_evt, PLAYBACK_STOPPED_EVENT_FLAG);
            continue;
        }

        if (evt & PLAYBACK_START_EVENT_FLAG)
        {
            if (inst_data->con[0].snk_data.audio_client)
            {
                USER_TRACE("bt_music: open again--\r\n");
                continue;
            }

            decode_data = play_data_decode(inst_data, &decode_len, 0);

            USER_TRACE("bt_music: open len=%d\r\n", decode_len);

            USER_TRACE("decode src_len:%d, dst_len:%d\n", inst_data->con[0].snk_data.pt_curdata->len, decode_len);
            if (decode_len == 0)
            {
                rt_thread_mdelay(5);
                rt_event_send(g_playback_evt, PLAYBACK_START_EVENT_FLAG);
                continue;
            }

            audio_parameter_t param = {0};
            if (inst_data->con[0].snk_data.codec == AV_SBC)
            {
                param.write_samplerate = inst_data->con[inst_data->con_idx].act_cfg.sample_freq;
            }
#ifdef CFG_AV_AAC
            else if (inst_data->con[0].snk_data.codec == AV_MPEG24_AAC)
            {
                param.write_samplerate = inst_data->con[inst_data->con_idx].act_aac_cfg.sample_freq;
            }
#endif
            else
            {
                USER_TRACE("Unsupported codec!!!!!\n");
                RT_ASSERT(0);
            }
            uint32_t origin_samplerate = param.write_samplerate;
            param.write_bits_per_sample = 16;
            param.write_channnel_num = 1;
            param.write_cache_size = 320 * g_drop_cnt + SPEAKER_DMA_SIZE;
            param.write_samplerate = 48000;
            debug_tx_cnt = 0;
            inst_data->con[0].snk_data.audio_client = audio_open(AUDIO_TYPE_BT_MUSIC, AUDIO_TX, &param, NULL, NULL);
            is_stopped = 0;
            if (!resample)
            {
                USER_TRACE("resample from %d to 48k", origin_samplerate);
                resample = sifli_resample_open(2, origin_samplerate, 48000);
                RT_ASSERT(resample);
                g_one_channle_size = 4096;
                g_left = (int16_t *)audio_mem_malloc(g_one_channle_size);
                g_right = (int16_t *)audio_mem_malloc(g_one_channle_size);
                RT_ASSERT(g_left && g_right);
            }
            g_remain = 0;
            g_offset = 0;
            prepare_ble_src_data((int16_t *)decode_data, decode_len);
            for (int i = 0; i < g_drop_cnt; i++)
            {
                audio_write(inst_data->con[0].snk_data.audio_client, (uint8_t *)zero, 160);
            }
        }
        if (evt & PLAYBACK_GETDATA_EVENT_FLAG)
        {
            if (debug_tx_cnt == 0)
            {
                //USER_TRACE("a2dp get data,total:%d,full:%d,empty:%d, curr %d\r\n", inst_data->con[0].snk_data.playlist.total_num,
                //           inst_data->con[0].snk_data.playlist.full_num, inst_data->con[0].snk_data.playlist.empty_num, inst_data->con[0].snk_data.playlist.cnt);
            }
            debug_tx_cnt++;

            if (is_stopped == 1 || inst_data->con[0].snk_data.play_state == FALSE || inst_data->con[0].snk_data.audio_client == NULL)
            {
                //USER_TRACE("snk: stop %d %d %x\r\n", is_stopped, inst_data->con[0].snk_data.play_state, inst_data->con[0].snk_data.audio_client);
                continue;
            }
        }

        if (g_remain < SPEAKER_DMA_SIZE)
        {
            memcpy(g_left, &g_left[g_offset], g_remain);
            memcpy(g_right, &g_right[g_offset], g_remain);
            decode_data = play_data_decode(inst_data, &decode_len, 0);
            if (decode_len == 0)
            {
                decode_len = 2560;
                decode_data = inst_data->con[0].snk_data.decode_buf;
                memset(decode_data, 0, decode_len);
            }
            prepare_ble_src_data((int16_t *)decode_data, decode_len);
        }

        ret_write = audio_write(inst_data->con[0].snk_data.audio_client, (uint8_t *)&g_left[g_offset], SPEAKER_DMA_SIZE);
        if (ret_write < 0)
        {
            USER_TRACE("playback write ret:%d\n", ret_write);
        }
        else if (ret_write == 0)
        {
            USER_TRACE("cache full\n");
        }
        else
        {
            if (rt_ringbuffer_space_len(ble_bap_src_enabled_ring) < SPEAKER_DMA_SIZE)
            {
                LOG_I("ble src cache full");
            }
            rt_ringbuffer_put(ble_bap_src_enabled_ring, (uint8_t *)&g_right[g_offset], SPEAKER_DMA_SIZE);
            g_offset += (SPEAKER_DMA_SIZE / 2);
            g_remain -= SPEAKER_DMA_SIZE;
        }
    }
}
#else
static void decode_playback_thread(void *args)
{
    bts2s_av_inst_data *inst_data;

    rt_uint32_t evt;
    play_data_t *pt_data;
    U8 *decode_data[MAX_ACTS] = {NULL};
    U16 decode_len[MAX_ACTS] = {0};
    U8  is_stopped[MAX_ACTS] = {1};
    U8  debug_tx_cnt[MAX_ACTS] = {0};
    int  ret_write = 0;
#if PKG_USING_VBE_DRC
    uint32_t vbe_out_size;
#endif
    g_playback_evt = rt_event_create("playback_evt", RT_IPC_FLAG_FIFO);
    U8 con_idx = 0;

    while (1)
    {
        evt = 0;
        rt_err_t err = rt_event_recv(g_playback_evt, PLAYBACK_GETDATA_EVENT_FLAG | PLAYBACK_START_EVENT_FLAG | PLAYBACK_STOPPING_EVENT_FLAG | PLAYBACK_START_EVENT_FLAG_EXT | PLAYBACK_GETDATA_EVENT_FLAG_EXT | PLAYBACK_STOPPING_EVENT_FLAG_EXT, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &evt);
        inst_data = bt_av_get_inst_data();
        if ((evt & PLAYBACK_STOPPING_EVENT_FLAG) || (evt & PLAYBACK_STOPPING_EVENT_FLAG_EXT))
        {
            if (evt & PLAYBACK_STOPPING_EVENT_FLAG)
                is_stopped[0] = 1;
            else
                is_stopped[1] = 1;
            rt_event_send(g_playback_evt, PLAYBACK_STOPPED_EVENT_FLAG);
            // continue;
        }

        if ((evt & PLAYBACK_START_EVENT_FLAG) || (evt & PLAYBACK_START_EVENT_FLAG_EXT))
        {
            if (evt & PLAYBACK_START_EVENT_FLAG)
                con_idx = 0;
            else if (evt & PLAYBACK_START_EVENT_FLAG_EXT)
                con_idx = 1;

            if (inst_data->con[con_idx].snk_data.audio_client)
            {
                USER_TRACE("bt_music: open again--\r\n");
                continue;
            }

            decode_data[con_idx] = play_data_decode(inst_data, &decode_len[con_idx], con_idx);
            USER_TRACE("(d%d)bt_music: open len=%d\r\n", con_idx, decode_len[con_idx]);

            USER_TRACE("(d%d)decode src_len:%d, dst_len:%d\n", con_idx, inst_data->con[con_idx].snk_data.pt_curdata->len, decode_len[con_idx]);
            if (decode_len[con_idx] == 0)
            {
                rt_thread_mdelay(3);
                if (con_idx == 0)
                    rt_event_send(g_playback_evt, PLAYBACK_START_EVENT_FLAG);
                else
                    rt_event_send(g_playback_evt, PLAYBACK_START_EVENT_FLAG_EXT);
            }
            else
            {
                audio_parameter_t param = {0};
                if (inst_data->con[con_idx].snk_data.codec == AV_SBC)
                {
                    param.write_samplerate = inst_data->con[con_idx].act_cfg.sample_freq;
                }
#ifdef CFG_AV_AAC
                else if (inst_data->con[con_idx].snk_data.codec == AV_MPEG24_AAC)
                {
                    param.write_samplerate = inst_data->con[con_idx].act_aac_cfg.sample_freq;
                }
#endif
                else
                {
                    USER_TRACE("Unsupported codec!!!!!\n");
                    RT_ASSERT(0);
                }
#ifdef A2DP_RELAY_SERVICE
                if (a2dp_relay_stereo_enable())
                {
                    param.write_channnel_num = 1;
                }
                else
#endif // A2DP_RELAY_SERVICE
                {
                    param.write_channnel_num = 2;
                }
                param.write_bits_per_sample = 16;
                param.write_cache_size = 8192;
                debug_tx_cnt[con_idx] = 0;
                inst_data->con[con_idx].snk_data.audio_client = audio_open(AUDIO_TYPE_BT_MUSIC, AUDIO_TX, &param, audio_bt_music_client_cb, (void *)&inst_data->con[con_idx].idx);
                is_stopped[con_idx] = 0;

                if (!resample)
                {
                    USER_TRACE("resample from %d to 48k", param.write_samplerate);
                    resample = sifli_resample_open(param.write_channnel_num, param.write_samplerate, 48000);
                    RT_ASSERT(resample);
                }

#if PKG_USING_VBE_DRC
                inst_data->con[con_idx].snk_data.vbe_out = rt_malloc(A2DP_VBE_OUT_BUFFER_SIZE);
                RT_ASSERT(inst_data->con[con_idx].snk_data.vbe_out);
                inst_data->con[con_idx].snk_data.vbe = vbe_drc_open(44100, 2, 16);
                vbe_out_size = vbe_drc_process(inst_data->con[con_idx].snk_data.vbe, (int16_t *)decode_data[con_idx], decode_len[con_idx] / 2, (int16_t *)inst_data->con[con_idx].snk_data.vbe_out, A2DP_VBE_OUT_BUFFER_SIZE);
#endif
            }
        }

        if (evt & PLAYBACK_GETDATA_EVENT_FLAG)
        {
            con_idx = 0;

            if (debug_tx_cnt[con_idx] % 125 == 0)
            {
                // USER_TRACE("(d%d)a2dp get data,total:%d,full:%d,empty:%d, curr %d\r\n", con_idx, inst_data->con[con_idx].snk_data.playlist.total_num,
                //            inst_data->con[con_idx].snk_data.playlist.full_num, inst_data->con[con_idx].snk_data.playlist.empty_num, inst_data->con[con_idx].snk_data.playlist.cnt);
            }
            debug_tx_cnt[con_idx]++;

            if (is_stopped[con_idx] == 1 || inst_data->con[con_idx].snk_data.play_state == FALSE || inst_data->con[con_idx].snk_data.audio_client == NULL)
            {
                //USER_TRACE("(d%d)snk: stop %d %d %x\r\n", con_idx, is_stopped[con_idx], inst_data->con[con_idx].snk_data.play_state, inst_data->con[con_idx].snk_data.audio_client);
                // continue;
            }
            else
            {
#if !defined(CFG_AV_AAC)
                if (decode_len[con_idx] == 0)
                {
                    decode_len[con_idx] = inst_data->con[con_idx].snk_data.decode_buf_len;
                    decode_data[con_idx] = inst_data->con[con_idx].snk_data.decode_buf;
                    memset(decode_data[con_idx], 0, decode_len[con_idx]);
                }
#else
                if (decode_len[con_idx] == 0)
                {
                    if (inst_data->con[con_idx].snk_data.codec == AV_SBC)
                    {
                        decode_len[con_idx] = inst_data->con[con_idx].snk_data.decode_buf_len;
                        decode_data[con_idx] = inst_data->con[con_idx].snk_data.decode_buf;
                        memset(decode_data[con_idx], 0, decode_len[con_idx]);
                    }
                    else
                    {
                        decode_data[con_idx] = play_data_decode(inst_data, &decode_len[con_idx], con_idx);
                    }
                }
#endif
            }
        }

        if (evt & PLAYBACK_GETDATA_EVENT_FLAG_EXT)
        {
            con_idx = 1;

            if (debug_tx_cnt[con_idx] % 125 == 0)
            {
                // USER_TRACE("(d%d)a2dp get data,total:%d,full:%d,empty:%d, curr %d\r\n", con_idx, inst_data->con[con_idx].snk_data.playlist.total_num,
                //            inst_data->con[con_idx].snk_data.playlist.full_num, inst_data->con[con_idx].snk_data.playlist.empty_num, inst_data->con[con_idx].snk_data.playlist.cnt);
            }
            debug_tx_cnt[con_idx]++;

            if (is_stopped[con_idx] == 1 || inst_data->con[con_idx].snk_data.play_state == FALSE || inst_data->con[con_idx].snk_data.audio_client == NULL)
            {
                //USER_TRACE("(d%d)snk: stop %d %d %x\r\n", con_idx, is_stopped[con_idx], inst_data->con[con_idx].snk_data.play_state, inst_data->con[con_idx].snk_data.audio_client);
                // continue;
            }
            else
            {
#if !defined(CFG_AV_AAC)
                if (decode_len[con_idx] == 0)
                {
                    decode_len[con_idx] = inst_data->con[con_idx].snk_data.decode_buf_len;
                    decode_data[con_idx] = inst_data->con[con_idx].snk_data.decode_buf;
                    memset(decode_data[con_idx], 0, decode_len[con_idx]);
                }
#else
                if (decode_len[con_idx] == 0)
                {
                    if (inst_data->con[con_idx].snk_data.codec == AV_SBC)
                    {
                        decode_len[con_idx] = inst_data->con[con_idx].snk_data.decode_buf_len;
                        decode_data[con_idx] = inst_data->con[con_idx].snk_data.decode_buf;
                        memset(decode_data[con_idx], 0, decode_len[con_idx]);
                    }
                    else
                    {
                        decode_data[con_idx] = play_data_decode(inst_data, &decode_len[con_idx], con_idx);
                    }
                }
#endif
            }
        }

        while ((decode_len[0] > 0) || (decode_len[1] > 0))
        {
#if PKG_USING_VBE_DRC
            ret_write = audio_write(inst_data->con[con_idx].snk_data.audio_client, inst_data->con[con_idx].snk_data.vbe_out, vbe_out_size);
#else

            if (get_server_current_device() == AUDIO_DEVICE_BLE_BAP_SINK)
            {
                int try_times = 0;
                uint32_t bytes = sifli_resample_process(resample, (int16_t *)decode_data[con_idx], decode_len[con_idx], 0);
                while (1)
                {
                    ret_write = audio_write(inst_data->con[con_idx].snk_data.audio_client, (uint8_t *)sifli_resample_get_output(resample), bytes);
                    if (ret_write)
                        break;
                    rt_thread_mdelay(10);
                    try_times++;
                    if (try_times > 20)
                        break;
                }
            }
            else
            {
                con_idx = 0;

                if (((evt & PLAYBACK_GETDATA_EVENT_FLAG) || (evt & PLAYBACK_START_EVENT_FLAG)) && (decode_len[con_idx] > 0))
                {
                    ret_write = audio_write(inst_data->con[con_idx].snk_data.audio_client, decode_data[con_idx], decode_len[con_idx]);

                    if (ret_write < 0)
                    {
                        USER_TRACE("playback write ret:%d\n", ret_write);
                        break;
                    }
                    else if (ret_write == 0)
                    {
                        if (get_server_current_device() == AUDIO_DEVICE_BLE_BAP_SINK)
                        {
                            USER_TRACE("--a2dp drop data\n");
                        }
                        break;
                    }
                }

                con_idx = 1;

                if (((evt & PLAYBACK_GETDATA_EVENT_FLAG_EXT) || (evt & PLAYBACK_START_EVENT_FLAG_EXT)) && (decode_len[con_idx] > 0))
                {
                    ret_write = audio_write(inst_data->con[con_idx].snk_data.audio_client, decode_data[con_idx], decode_len[con_idx]);

                    if (ret_write < 0)
                    {
                        USER_TRACE("playback write ret:%d\n", ret_write);
                        break;
                    }
                    else if (ret_write == 0)
                    {
                        if (get_server_current_device() == AUDIO_DEVICE_BLE_BAP_SINK)
                        {
                            USER_TRACE("--a2dp drop data\n");
                        }
                        break;
                    }
                }
            }
#endif

            {
                con_idx = 0;
                if (((evt & PLAYBACK_GETDATA_EVENT_FLAG) || (evt & PLAYBACK_START_EVENT_FLAG)) && (inst_data->con[con_idx].snk_data.play_state))
                {
                    decode_data[con_idx] = play_data_decode(inst_data, &decode_len[con_idx], con_idx);

#if PKG_USING_VBE_DRC
                    vbe_out_size = vbe_drc_process(inst_data->con[con_idx].snk_data.vbe, (int16_t *)decode_data[con_idx], decode_len[con_idx] / 2, (int16_t *)inst_data->con[con_idx].snk_data.vbe_out, A2DP_VBE_OUT_BUFFER_SIZE);
#endif
                }

                con_idx = 1;
                if (((evt & PLAYBACK_GETDATA_EVENT_FLAG_EXT) || (evt & PLAYBACK_START_EVENT_FLAG_EXT)) && (inst_data->con[con_idx].snk_data.play_state))
                {
                    decode_data[con_idx] = play_data_decode(inst_data, &decode_len[con_idx], con_idx);

#if PKG_USING_VBE_DRC
                    vbe_out_size = vbe_drc_process(inst_data->con[con_idx].snk_data.vbe, (int16_t *)decode_data[con_idx], decode_len[con_idx] / 2, (int16_t *)inst_data->con[con_idx].snk_data.vbe_out, A2DP_VBE_OUT_BUFFER_SIZE);
#endif
                }

                //!If two connections are playing music, you should pay attention to switching decoding in time
                //?Can be optimized
                if (bt_av_get_sink_streaming_number() == MAX_ACTS)
                {
                    if (!((evt & PLAYBACK_GETDATA_EVENT_FLAG) && (evt & PLAYBACK_GETDATA_EVENT_FLAG_EXT)))
                    {
                        break;
                    }
                }

                if ((evt & PLAYBACK_STOPPING_EVENT_FLAG) || (evt & PLAYBACK_STOPPING_EVENT_FLAG_EXT))
                {
                    break;
                }
            }
        }
    }
}
#endif

#ifdef CFG_AV_AAC
    #define DEPLAYBACK_STACK_SIZE   (1024 * 16)
#else
    #if defined(__GNUC__)
        #define DEPLAYBACK_STACK_SIZE   2048
    #else
        #define DEPLAYBACK_STACK_SIZE   1024
    #endif
#endif

#if defined(SF32LB52X)

    static rt_uint8_t deplayback_thread_stack[DEPLAYBACK_STACK_SIZE];
    static struct rt_thread deplayback_thread;
#endif
static int audio_decode_thread_init(void)
{

#if defined(SF32LB52X)
    rt_thread_init(&deplayback_thread,
                   "deplayback_th",
                   decode_playback_thread,
                   RT_NULL,
                   &deplayback_thread_stack[0],
                   sizeof(deplayback_thread_stack),
                   RT_THREAD_PRIORITY_HIGH,
                   RT_THREAD_TICK_DEFAULT);
    g_playback_thread = &deplayback_thread;
#else
    g_playback_thread = rt_thread_create("deplayback_th", decode_playback_thread, NULL, DEPLAYBACK_STACK_SIZE, RT_THREAD_PRIORITY_HIGH, RT_THREAD_TICK_DEFAULT);

#endif
    if (g_playback_thread)
    {
        rt_thread_startup(g_playback_thread);
    }
    else
    {
        USER_TRACE("deplayback_th thread fail \n");
    }


    return 0;
}
INIT_PRE_APP_EXPORT(audio_decode_thread_init);


static void stop_audio_playback(bts2s_av_inst_data *inst, U8 con_idx)
{
    rt_uint32_t evt;

    USER_TRACE("(d%d)stop_audio_playback state:%d\n", con_idx, inst->con[con_idx].snk_data.play_state);
    if (inst->con[con_idx].snk_data.play_state == TRUE)
    {
#ifdef A2DP_RELAY_SERVICE
        a2dp_relay_stop();
#endif // A2DP_RELAY_SERVICE
        if (con_idx == 0)
            rt_event_send(g_playback_evt, PLAYBACK_STOPPING_EVENT_FLAG);
        else
            rt_event_send(g_playback_evt, PLAYBACK_STOPPING_EVENT_FLAG_EXT);
        rt_event_recv(g_playback_evt, PLAYBACK_STOPPED_EVENT_FLAG, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &evt);
        inst->con[con_idx].snk_data.play_state = FALSE;
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
        sifli_resample_close(resample);
        resample = NULL;
#if BT_BAP_BROADCAST_SOURCE
        audio_mem_free(g_left);
        g_left = NULL;
        audio_mem_free(g_right);
        g_right = NULL;
#endif
        audio_close(inst->con[con_idx].snk_data.audio_client);
        inst->con[con_idx].snk_data.audio_client = NULL;
#endif
#if PKG_USING_VBE_DRC
        vbe_drc_close(inst->con[con_idx].snk_data.vbe);
        rt_free(inst->con[con_idx].snk_data.vbe_out);
        inst->con[con_idx].snk_data.vbe = NULL;
        inst->con[con_idx].snk_data.vbe_out = NULL;
#endif
    }

    list_all_free(&(inst->con[con_idx].snk_data.playlist), con_idx);

    if (inst->con[con_idx].snk_data.decode_buf)
    {
        // frm payload could be updated before decode
        frms_per_payload[con_idx] = 0;
        bfree(inst->con[con_idx].snk_data.decode_buf);
        inst->con[con_idx].snk_data.decode_buf = NULL;
    }

}

static void stop_audio_playback_temporarily(bts2s_av_inst_data *inst, U8 con_idx)
{
    rt_uint32_t evt;

    USER_TRACE("(d%d)stop_audio_playback_temporarily state:%d\n", con_idx, inst->con[con_idx].snk_data.play_state);
    if (inst->con[con_idx].snk_data.play_state == TRUE)
    {
        if (con_idx == 0)
            rt_event_send(g_playback_evt, PLAYBACK_STOPPING_EVENT_FLAG);
        else
            rt_event_send(g_playback_evt, PLAYBACK_STOPPING_EVENT_FLAG_EXT);
        rt_event_recv(g_playback_evt, PLAYBACK_STOPPED_EVENT_FLAG, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &evt);
        inst->con[con_idx].snk_data.play_state = FALSE;
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
        sifli_resample_close(resample);
        resample = NULL;
#if BT_BAP_BROADCAST_SOURCE
        audio_mem_free(g_left);
        g_left = NULL;
        audio_mem_free(g_right);
        g_right = NULL;
#endif
        audio_close(inst->con[con_idx].snk_data.audio_client);
        inst->con[con_idx].snk_data.audio_client = NULL;
#endif
#if PKG_USING_VBE_DRC
        vbe_drc_close(inst->con[con_idx].snk_data.vbe);
        rt_free(inst->con[con_idx].snk_data.vbe_out);
        inst->con[con_idx].snk_data.vbe = NULL;
        inst->con[con_idx].snk_data.vbe_out = NULL;
#endif

    }

    list_all_free(&(inst->con[con_idx].snk_data.playlist), con_idx);

}
#endif

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_a2dp_source_connect_request(BTS2S_BD_ADDR *bd)
{
    BTS2S_BD_ADDR temp = {0xffffff, 0xff, 0xffff};
    bt_err_t res = BT_EOK;

    if (!bd_eq(bd, &temp))
    {
        bt_av_conn(bd, AV_SRC);
        USER_TRACE(">> av snk connect\n");
    }
    else
    {
        res = BT_ERROR_INPARAM;
        USER_TRACE(">> pls input remote device address\n");
    }

    return res;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_close_a2dp(void)
{
    bts2s_av_inst_data *inst_data;
    inst_data = bt_av_get_inst_data();

    inst_data->close_pending = TRUE;
    for (uint32_t i = 0; i < MAX_CONNS; i++)
    {
        if (inst_data->con[i].cfg == AV_AUDIO_SNK)
            bt_av_disconnect(i);
    }
}

void bt_avsnk_disc_2_src(void)
{
    bts2s_av_inst_data *inst_data;
    inst_data = bt_av_get_inst_data();

    for (uint32_t i = 0; i < MAX_CONNS; i++)
    {
        if (inst_data->con[i].cfg == AV_AUDIO_SNK)
            bt_av_disconnect(i);
    }
}

void bt_avsnk_disc_by_addr(BTS2S_BD_ADDR *bd_addr)
{
    bts2s_av_inst_data *inst_data;
    inst_data = bt_av_get_inst_data();

    for (uint32_t i = 0; i < MAX_CONNS; i++)
    {
        if (bd_eq(bd_addr, &inst_data->con[i].av_rmt_addr))
        {
            bt_av_disconnect(i);
            break;
        }
    }
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_suspend_stream(bts2_app_stru *bts2_app_data)
{
    for (uint32_t i = 0; i < MAX_CONNS; i++)
        bt_av_suspend_stream(i);
}
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_start_stream(bts2_app_stru *bts2_app_data)
{
    for (uint32_t i = 0; i < MAX_CONNS; i++)
        bt_av_start_stream(i);
}


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_release_stream(bts2_app_stru *bts2_app_data)
{
    for (uint32_t i = 0; i < MAX_CONNS; i++)
        bt_av_release_stream(i);
}

#if 0
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2s_avsnk_inst_data *inst:
 *
 * OUTPUT:
 *      int.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
static int bt_avsnk_get_plyback_conn(bts2s_avsnk_inst_data *inst)
{
    U8 found = FALSE;
    U8 i = 0;
    int res = - 1;

    for (i = 0; i < MAX_CONNS; i++)
    {
        if ((inst->local_seid_info[inst->con[i].local_seid_idx].local_seid.sep_type == AV_SNK) &&
                (inst->local_seid_info[inst->con[i].local_seid_idx].local_seid.in_use == TRUE))
        {
            found = TRUE;
            break;
        }
    }
    if (found)
    {
        res = i;
    }
    return res;
}
#endif
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_sbc_cfg *cfg:
 *      U8 *sbc_ie
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
static void bt_avsnk_init_data(bts2s_avsnk_inst_data *inst, bts2_app_stru *bts2_app_data)
{
    inst->playlist.cnt = 0;
    inst->playlist.cnt_th = SINK_DATA_LIST_START_THRESHOLD;
    inst->playlist.first = NULL;
    inst->playlist.last = NULL;
    inst->play_state = FALSE;
#ifndef RT_USING_UTEST
    inst->slience_filter_enable = 1;
    inst->slience_count = 0;
#endif
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
    inst->audio_client = NULL;
#endif
    inst->decode_buf = NULL;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      U16 r1:
 *      U8 r2:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
static U8 bt_avsnk_role_type_equal(U16 r1, U8 r2)
{
    U8 res;

    if ((r1 == AV_AUDIO_SRC) && (r2 == AV_SRC))
    {
        res = TRUE;
    }
    else if ((r1 == AV_AUDIO_SNK) && (r2 == AV_SNK))
    {
        res = TRUE;
    }
    else
    {
        res = FALSE;
    }
    return res;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      U8 con_idx:
 *
 * OUTPUT:
 *      U8.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U8 bt_avsnk_prepare_sbc(bts2s_av_inst_data *inst, U8 con_idx)
{
    U8 res;
    U8 bit_pool_alt;
    U8 toggle_period;
    bts2_sbc_cfg *cfg = &inst->con[con_idx].act_cfg;
    cfg->bit_pool = bts2_sbc_calc_bit_pool(&bit_pool_alt,
                                           &toggle_period,
                                           cfg->chnl_mode,
                                           cfg->sample_freq,
                                           cfg->blocks,
                                           cfg->subbands,
                                           BIT_RATE_DEAFLUT);

    if (cfg->bit_pool > cfg->max_bitpool)
    {
        cfg->bit_pool = cfg->max_bitpool;
    }

    res = bts2_sbc_decode_cfg_ext(cfg->chnl_mode,
                                  cfg->alloc_method,
                                  cfg->sample_freq,
                                  cfg->blocks,
                                  cfg->subbands,
                                  cfg->bit_pool,
                                  con_idx);
    return res;
}

U16 bt_avsnk_calculate_decode_buffer_size(bts2s_av_inst_data *inst, U8 con_idx)
{
    bts2_sbc_cfg *cfg = &inst->con[con_idx].act_cfg;
    U16 sbc_frame_size;
    U16 decode_buffer_size;
    U16 pcm_samples_per_sbc_frame;

    sbc_frame_size = bts2_sbc_calculate_framelen(cfg->chnl_mode,
                     cfg->blocks,
                     cfg->subbands,
                     cfg->bit_pool);
    frms_per_payload[con_idx] = (U16)((AV_MTU_SIZE - 14) / sbc_frame_size + 1);
    pcm_samples_per_sbc_frame = bts2_sbc_calculate_pcm_samples_per_sbc_frame(cfg->blocks, cfg->subbands);
    decode_buffer_size = pcm_samples_per_sbc_frame * frms_per_payload[con_idx] * 2;
    USER_TRACE("(d%d)sbc_frame_size = %d,pcm_samples_per_sbc_frame = %d,decode_buffer_size = %d\n", con_idx,
               sbc_frame_size, pcm_samples_per_sbc_frame, decode_buffer_size);
    return decode_buffer_size;
}


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_hdl_disc_handler(bts2s_av_inst_data *inst, uint8_t con_idx)
{
    U8 codec = inst->local_seid_info[inst->con[con_idx].local_seid_idx].local_seid.codec;
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
    stop_audio_playback(inst, con_idx);
#endif
    if (codec == AV_SBC)
        bts2_sbc_decode_completed_ext(con_idx);

#ifdef CFG_AV_AAC
    if ((codec == AV_MPEG24_AAC) && is_aac_inited[con_idx])
    {
        is_aac_inited[con_idx] = 0;
        NeAACDecClose(hAac[con_idx]);
        hAac[con_idx] = NULL;
    }
#endif

}


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
int8_t bt_avsnk_hdl_start_cfm(bts2s_av_inst_data *inst, uint8_t con_idx)
{
    int8_t ret = -1;
    if (!bt_avsnk_prepare_sbc(inst, con_idx))
        ret = 0;
    return ret;
}
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/

void bt_avsnk_close_handler(bts2s_av_inst_data *inst, uint8_t con_idx)
{
    U8 codec = inst->local_seid_info[inst->con[con_idx].local_seid_idx].local_seid.codec;
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
    stop_audio_playback(inst, con_idx);
#endif

#ifdef CFG_AV_AAC
    if ((codec == AV_MPEG24_AAC) && is_aac_inited[con_idx])
    {
        is_aac_inited[con_idx] = 0;
        NeAACDecClose(hAac[con_idx]);
        hAac[con_idx] = NULL;
    }
#endif
}

void bt_avsnk_abort_handler(bts2s_av_inst_data *inst, uint8_t con_idx)
{
    U8 codec = inst->local_seid_info[inst->con[con_idx].local_seid_idx].local_seid.codec;
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
    stop_audio_playback(inst, con_idx);
#endif

#ifdef CFG_AV_AAC
    if ((codec == AV_MPEG24_AAC) && is_aac_inited[con_idx])
    {
        is_aac_inited[con_idx] = 0;
        NeAACDecClose(hAac[con_idx]);
        hAac[con_idx] = NULL;
    }
#endif
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
uint8_t bt_avsnk_hdl_start_ind(bts2s_av_inst_data *inst, BTS2S_AV_START_IND *msg, uint8_t con_idx)
{
    U8 res;
    U8 i;
    U8 shdl = 0;
    U8 codec;

    codec = inst->local_seid_info[inst->con[con_idx].local_seid_idx].local_seid.codec;

    res = AV_ACPT;

    for (i = 0; i < msg->list_len; i++)
    {
        shdl = *(&msg->first_shdl + i);

        if (shdl == inst->con[con_idx].stream_hdl)
        {
            if (inst->con[con_idx].st == avconned_open)
            {
                if (codec == AV_SBC)
                {
                    //?Redundant code????
                    // if (!bt_avsnk_prepare_sbc(inst, con_idx))
                    // {
                    //     break;
                    // }
                }
                else
                {
#ifdef CFG_AV_AAC
                    USER_TRACE("aac open\n");
                    hAac[con_idx] = NeAACDecOpen();
                    is_aac_inited[con_idx] = 0;
                    // Get the current config
                    NeAACDecConfigurationPtr conf =
                        NeAACDecGetCurrentConfiguration(hAac[con_idx]);
                    conf->defObjectType = LC;
                    conf->outputFormat = FAAD_FMT_16BIT;
                    conf->downMatrix = 1;
                    conf->useOldADTSFormat = 0;
                    // If needed change some of the values in conf
                    //
                    // Set the new configuration
                    NeAACDecSetConfiguration(hAac[con_idx], conf);
#endif
                }
            }
            else
            {
                res = AV_BAD_ST;
                break;
            }
        }
        else
        {
            res = AV_BAD_ACP_SEID;
            break;
        }
    }

    if ((res == AV_ACPT) && (i == msg->list_len))
    {
        USER_TRACE(">> (d%d)accept to start the stream\n", con_idx);

        inst->con[con_idx].snk_data.codec = codec;
#ifdef A2DP_RELAY_SERVICE
        // 2 means stereo, 0 means left, 1 means right
        uint8_t target_channel = a2dp_relay_stereo_enable() ? (a2dp_relay_get_channel() == 0 ? 1 : 0) : 2;
        LOG_D("src channel %d, snk %d", a2dp_relay_get_channel(), target_channel);
        a2dp_relay_start(codec, inst->con[con_idx].act_cfg.chnl_mode, inst->con[con_idx].act_cfg.alloc_method,
                         inst->con[con_idx].act_cfg.sample_freq, inst->con[con_idx].act_cfg.blocks,
                         inst->con[con_idx].act_cfg.subbands, inst->con[con_idx].act_cfg.bit_pool,
                         SINK_DATA_LIST_START_THRESHOLD, SINK_DATA_LIST_MAX_THRESHOLD, target_channel);
#endif // A2DP_RELAY_SERVICE
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
        if (inst->con[con_idx].snk_data.codec == AV_SBC)
        {
            if (inst->con[con_idx].snk_data.decode_buf == NULL)
            {
                U16 decode_buffer_size;
                decode_buffer_size = bt_avsnk_calculate_decode_buffer_size(inst, con_idx);
                USER_TRACE("decode_buffer_size = %d\n", decode_buffer_size);
                inst->con[con_idx].snk_data.decode_buf = bmalloc(decode_buffer_size);
                inst->con[con_idx].snk_data.decode_buf_len = decode_buffer_size;
            }
            BT_OOM_ASSERT(inst->con[con_idx].snk_data.decode_buf);
        }
        list_all_free(&(inst->con[con_idx].snk_data.playlist), con_idx);
#endif
    }

    return res;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_hdl_suspend_ind(bts2s_av_inst_data *inst, uint8_t con_idx)
{

    U8 codec = inst->local_seid_info[inst->con[con_idx].local_seid_idx].local_seid.codec;
    USER_TRACE(">> accept to suspend the av stream\n");
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
    stop_audio_playback(inst, con_idx);
#endif

#ifdef CFG_AV_AAC
    if ((codec == AV_MPEG24_AAC) && is_aac_inited[con_idx])
    {
        is_aac_inited[con_idx] = 0;
        NeAACDecClose(hAac[con_idx]);
        hAac[con_idx] = NULL;
    }
#endif
}

#ifdef A2DP_RELAY_SERVICE
__WEAK U16 a2dp_relay_transfer(U8 *data, U16 len, uint8_t left_cnt, uint32_t left_byte)
{
    return 0;
}

__WEAK void a2dp_relay_trigger_audio_server(void)
{

}

__WEAK void a2dp_relay_stop(void)
{

}

__WEAK uint8_t a2dp_relay_stereo_enable(void)
{
    return 0;
}

__WEAK uint8_t a2dp_relay_get_channel(void)
{
    return 0;
}

__WEAK void a2dp_relay_start(U8 codec, BTS2E_SBC_CHNL_MODE chnl_mode, BTS2E_SBC_ALLOC_METHOD alloc_method, U16 sample_freq,
                             U8 nrof_blocks, U8 nrof_subbands, U8 bitpool, U8 start_playing_cnt, U8 max_playing_cnt, U8 target_channel)
{

}
#endif // A2DP_RELAY_SERVICE

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_hdl_streamdata_ind(bts2s_av_inst_data *inst, uint8_t con_idx, BTS2S_AV_STREAM_DATA_IND *msg)
{
    int ret = -1;
    U8 *frm_ptr;
    U8 codec;
    U16 sbc_frame_size;
    static uint32_t count = 0;

    codec = inst->local_seid_info[inst->con[con_idx].local_seid_idx].local_seid.codec;


    //!Only applicable to SBC codec
    if (bt_av_get_slience_filter_enable(con_idx) && (codec == AV_SBC))
    {
        if (msg->len <= (AV_FIXED_MEDIA_PKT_HDR_SIZE + 1))
        {
            USER_TRACE("a2dp play decode len:%d\n", msg->len);
            bfree(msg->data);
            return;
        }

        frm_ptr = msg->data + AV_FIXED_MEDIA_PKT_HDR_SIZE + 1;
        U8 bitpool = *(frm_ptr + 2);;
        bts2_sbc_cfg *cfg = &inst->con[con_idx].act_cfg;

        sbc_frame_size = bts2_sbc_calculate_framelen(cfg->chnl_mode,
                         cfg->blocks,
                         cfg->subbands,
                         bitpool);

        if (0 == memcmp(frm_ptr, frm_ptr + sbc_frame_size, sbc_frame_size))
        {
            inst->con[con_idx].snk_data.slience_count++;
            // USER_TRACE("sbc mute count = %d\n",inst->con[con_idx].snk_data.slience_count);
            if (inst->con[con_idx].snk_data.slience_count > SINK_DATA_LIST_MAX_THRESHOLD + 2)
            {
                if (inst->con[con_idx].snk_data.play_state  == TRUE)
                {
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
                    stop_audio_playback(inst, con_idx);
#endif
                }
                bfree(msg->data);
                return;
            }
            else
            {
                if (inst->con[con_idx].snk_data.play_state  == FALSE)
                {
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
                    list_all_free(&(inst->con[con_idx].snk_data.playlist), con_idx);
#endif
                    bfree(msg->data);
                    return;
                }
            }
        }
        else
        {
#ifdef A2DP_RELAY_SERVICE
            // resume
            if (inst->snk_data.slience_count > (SINK_DATA_LIST_MAX_THRESHOLD + 2))
            {
                LOG_I("silence start");
                a2dp_relay_start(inst->snk_data.codec, inst->con[con_idx].act_cfg.chnl_mode, inst->con[con_idx].act_cfg.alloc_method,
                                 inst->con[con_idx].act_cfg.sample_freq, inst->con[con_idx].act_cfg.blocks,
                                 inst->con[con_idx].act_cfg.subbands, inst->con[con_idx].act_cfg.bit_pool,
                                 SINK_DATA_LIST_START_THRESHOLD, SINK_DATA_LIST_MAX_THRESHOLD, 0);
            }
#endif // A2DP_RELAY_SERVICE
            inst->con[con_idx].snk_data.slience_count = 0;
        }
    }

    {
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
        play_data_t *pt_data = (play_data_t *)msg->data;
        uint8_t ret;

#ifdef A2DP_RELAY_SERVICE
        uint32_t left_byte = 0;
        if (inst->snk_data.audio_client)
        {
            uint8_t left_cnt = inst->snk_data.playlist.cnt;
            audio_ioctl(inst->snk_data.audio_client, 3, &left_byte);
            LOG_I("cnt[%d] %d, %d", count++, left_cnt, left_byte);
            a2dp_relay_transfer(msg->data, msg->len, left_cnt, left_byte);
        }
#endif // A2DP_RELAY_SERVICE
        pt_data->len = msg->len;
        ret = list_push_back(&inst->con[con_idx].snk_data.playlist, &(pt_data->hdr), con_idx);
        if ((inst->con[con_idx].snk_data.play_state == FALSE) && (ret == 1))
        {
#ifdef A2DP_RELAY_SERVICE
            a2dp_relay_trigger_audio_server();
#endif // A2DP_RELAY_SERVICE
            inst->con[con_idx].snk_data.play_state = TRUE;
            USER_TRACE("av_snk.c open a2dp\r\n");
            if (con_idx == 0)
                rt_event_send(g_playback_evt, PLAYBACK_START_EVENT_FLAG);
            else
                rt_event_send(g_playback_evt, PLAYBACK_START_EVENT_FLAG_EXT);
        }
        else if (ret == 2)
        {
            list_hdr_t *hdr;
            hdr = list_pop_front(&inst->con[con_idx].snk_data.playlist, con_idx);
            bfree(hdr);

            //?Need to re-trigger?
            if (con_idx == 0)
                rt_event_send(g_playback_evt, PLAYBACK_GETDATA_EVENT_FLAG);
            else
                rt_event_send(g_playback_evt, PLAYBACK_GETDATA_EVENT_FLAG_EXT);
        }
#else
        bfree(msg->data);
#endif
    }


}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_init(bts2s_av_inst_data *inst, bts2_app_stru *bts2_app_data)
{
    for (U8 i = 0; i < MAX_ACTS; i++)
    {
        bt_avsnk_init_data(&inst->con[i].snk_data, bts2_app_data);
        //inst->play_handle = NULL;
        inst->con[i].snk_data.buf_sem = rt_sem_create("bt_av_sink", 1, RT_IPC_FLAG_FIFO);
        //inst->audio_len = 0;
    }

#ifdef CFG_OPEN_AVSNK
    bts2s_avsnk_openFlag = 1;
#else
    bts2s_avsnk_openFlag = 0;
#endif

    if (1 == bts2s_avsnk_openFlag)
    {
        INFO_TRACE(">> AV sink enabled\n");
        av_enb_req(inst->que_id, AV_AUDIO_SNK); //act the svc
    }
}

void bt_avsnk_open(bts2s_av_inst_data *inst)
{
    USER_TRACE("bt_avsnk_open %d flag\n", bts2s_avsnk_openFlag);
    if (0 == bts2s_avsnk_openFlag)
    {
        bts2s_avsnk_openFlag = 0x01;
        av_enb_req(inst->que_id, AV_AUDIO_SNK); //act the svc
    }
    else
    {

        bt_interface_bt_event_notify(BT_NOTIFY_A2DP, BT_NOTIFY_AVSNK_OPEN_COMPLETE, NULL, 0);
        INFO_TRACE(">> have open,ura AV sink open\n");
    }
}
void bt_avsnk_close(bts2s_av_inst_data *inst)
{
    USER_TRACE("bt_avsnk_close %d flag\n", bts2s_avsnk_openFlag);
    if (0x01 == bts2s_avsnk_openFlag)
    {
        bts2s_avsnk_openFlag = 0x00;
        av_disb_req(); //disable the svc
    }
    else
    {
        bt_interface_bt_event_notify(BT_NOTIFY_A2DP, BT_NOTIFY_AVSNK_CLOSE_COMPLETE, NULL, 0);
        INFO_TRACE(">> have close,urc AV sink close\n");
    }
}

void bt_avsnk_rel(bts2s_av_inst_data *inst)
{
    for (U8 i = 0; i < MAX_CONNS; i++)
        rt_sem_delete(inst->con[i].snk_data.buf_sem);
}

#if 0
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
static void bt_avsnk_hdl_sts_ind(bts2_app_stru *bts2_app_data)
{
    BTS2S_AV_STS_IND *msg;

    msg = (BTS2S_AV_STS_IND *)bts2_app_data->recv_msg;
    switch (msg->st_type)
    {
    case BTS2MU_AV_ENB_CFM:
        bt_avsnk_hdl_enb_cfm(bts2_app_data);
        break;

    case BTS2MU_AV_DISB_CFM:
        //hdl_AV_DISB_CFM(bts2_app_data);
        break;

    case BTS2MU_AV_CONN_CFM:
        bt_avsnk_hdl_conn_cfm(bts2_app_data);
        break;

    case BTS2MU_AV_CONN_IND:
        bt_avsnk_hdl_conn_ind(bts2_app_data);
        break;

    default:
        break;
    }
}
#endif

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_msg_handler(bts2_app_stru *bts2_app_data)
{
    U16 *msg_type;
    msg_type = (U16 *)bts2_app_data->recv_msg;
    switch (*msg_type)
    {
    case BTS2MU_AV_SECU_CTRL_CFM:
    {
        INFO_TRACE("<< receive av sercurity control confirmation \n");
        break;
    }
    case BTS2MU_AV_SECU_CTRL_IND:
    {
        INFO_TRACE("<< receive av security control indication\n");
        break;
    }
#if 0
    case BTS2MU_AV_STS_IND:
    {
        bt_avsnk_hdl_sts_ind(bts2_app_data);
        break;
    }
#endif
    default:
        break;
    }
}

#if 0
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_recfg(bts2_app_stru *bts2_app_data)
{
    bts2s_avsnk_inst_data *inst;
    U8 app_serv_cap[SBC_MEDIA_CODEC_SC_SIZE];
    U8 i;
    inst = global_inst;
    if (inst->con[inst->con_idx].st == avconned_open)
    {
        app_serv_cap[0] = AV_SC_MEDIA_CODEC;
        app_serv_cap[1] = SBC_MEDIA_CODEC_SC_SIZE - 2;
        app_serv_cap[2] = AV_AUDIO << 4;
        app_serv_cap[3] = AV_SBC;
        for (i = 0; i < SBC_MEDIA_CODEC_SC_SIZE - 4; i++)
        {
            app_serv_cap[4 + i] = avsnk_sbc_cfg[1][i];/*choose the other sbc code mothod*/
        }
        bt_avsnk_store_sbc_cfg(&inst->con[inst->con_idx].act_cfg, app_serv_cap + 4);
        USER_TRACE(">> reset the configuration of the stream\n");
        av_recfg_req(inst->con[inst->con_idx].stream_hdl, ASSIGN_TLABEL, SBC_MEDIA_CODEC_SC_SIZE, app_serv_cap);
    }
    else
    {
        if (inst->con[inst->con_idx].st == avconned_streaming)
        {
            USER_TRACE(" -- the stream is streaming, please supped it!\n");
        }
        else
        {
            USER_TRACE(" -- the stream is not in use!\n");
        }
    }
}
#endif
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_abort_stream(bts2_app_stru *bts2_app_data)
{
    for (uint32_t i = 0; i < MAX_CONNS; i++)
        bt_av_abort_stream(i);
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_get_cfg(bts2_app_stru *bts2_app_data)
{
    for (uint32_t i = 0; i < MAX_CONNS; i++)
        bt_av_get_cfg(i);
}


#endif


