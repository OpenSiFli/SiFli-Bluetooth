/*
 * SPDX-FileCopyrightText: 2025-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <stdlib.h>

#ifdef A2DP_RELAY_SERVICE

#include "a2dp_relay.h"
#include "bf0_ble_gap.h"
#include "bf0_sibles.h"
#include "att.h"
#include "bf0_sibles_advertising.h"
#include "bf0_sibles_serial_trans_service.h"
#include "av_sbc_api.h"
#include "bt_connection_manager.h"
#include "bts2_app_inc.h"

#ifdef AUDIO_USING_MANAGER
    #include "audio_server.h"
#endif // AUDIO_USING_MANAGER

#define LOG_TAG "a2_rly"

#include "log.h"


typedef enum
{
    A2DP_RELAY_STATE_IDLE,
    A2DP_RELAY_STATE_CONN,
    A2DP_RELAY_STATE_CODEC_OPENED,
    A2DP_RELAY_STATE_PLAYING,
} A2DP_RELAY_STATE_t;

typedef struct
{
    int32_t diff_total;
    uint16_t track_cnt;
    uint8_t err_cnt;
} a2dp_relay_pll_ctrl_t;

typedef struct
{
    uint8_t role;
    uint8_t snk_channel; // mono or stereo
    uint8_t chnl_nb;
    A2DP_RELAY_STATE_t relay_state;
    uint8_t is_advertising;
    uint8_t relay_preparing;
    uint32_t extra_zero_byte;
    uint8_t handle;
    uint8_t codec;
    uint8_t frms_per_payload;
    uint8_t max_playing_cnt;
    a2dp_relay_callback callback;
    a2dp_relay_sbc_para_t sbc_para;
    play_list_t playlist;
    play_data_t *pt_curdata;
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
    a2dp_relay_pll_ctrl_t pll_ctrl;
    audio_client_t audio_client;
#endif
    uint8_t  *decode_buf;
    uint16_t decode_buf_len;
    rt_mutex_t start_mux;
    rt_sem_t buf_sem;
} a2dp_relay_sink_env_t;


static rt_event_t g_playback_evt;
static rt_thread_t g_playback_thread = NULL;

static a2dp_relay_sink_env_t g_a2dp_relay_sink_env;

#define BEGIN_ACCESS_BUF()              rt_sem_take(g_a2dp_relay_sink_env.buf_sem, RT_WAITING_FOREVER)
#define END_ACCESS_BUF()                rt_sem_release(g_a2dp_relay_sink_env.buf_sem)
#define PLAY_LIST_MAX                   (g_a2dp_relay_sink_env.max_playing_cnt)

#define PCM_DATA_CAL(delta_ms, freq, nb_chan)             (((uint32_t)freq * nb_chan * 2 * delta_ms)/1000)
#define PCM_TIME_CAL(delta_byte, freq, nb_chan)           ((uint32_t)delta_byte * 1000 / (freq * nb_chan * 2))

/********************************************
* Internal APIs
*
*********************************************/

static a2dp_relay_sink_env_t *a2dp_relay_sink_get_env(void)
{
    return &g_a2dp_relay_sink_env;
}


static uint8_t list_push_back(play_list_t *list, list_hdr_t *hdr)
{
    uint8_t ret = 0;

    RT_ASSERT(hdr != NULL);

    BEGIN_ACCESS_BUF();

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
    //ret = (list->cnt >= list->cnt_th) ? 1 : 0;
    ret = list->cnt;
    //RT_ASSERT(list->cnt <= 100);
    if (list->cnt > PLAY_LIST_MAX)
    {
        //USER_TRACE("list->cnt= %d\n", list->cnt);
        list->full_num++;
    }
    list->total_num++;
    END_ACCESS_BUF();

    return ret;
}


static list_hdr_t *list_pop_front_n(play_list_t *list, uint8_t n)
{
    list_hdr_t *hdr;
    if (n == 0)
        return NULL;


    while (n--)
    {
        BEGIN_ACCESS_BUF();
        hdr = list->first;
        if (hdr != NULL)
        {
            list->first = list->first->next;
            list->cnt--;
            if (list->first == NULL)
            {
                list->last = NULL;
                END_ACCESS_BUF();
                break;
            }

        }
        else
        {
            END_ACCESS_BUF();
            break;
        }
        END_ACCESS_BUF();
        free(hdr);
    }


    return hdr;
}

static list_hdr_t *list_pop_front(play_list_t *list)
{
    list_hdr_t *hdr;

    BEGIN_ACCESS_BUF();

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

    END_ACCESS_BUF();

    return hdr;
}

static void list_all_free(play_list_t *list)
{
    list_hdr_t *hdr;

    LOG_I("all free:%x,cnt:%d\n", list->first, list->cnt);

    list_pop_front_n(list, list->cnt);
    list->empty_num = 0;
    list->full_num = 0;
    list->total_num = 0;
}

static uint8_t *play_data_decode_data_process(a2dp_relay_sink_env_t *env, U8 *decode_buf, U16 *out_len)
{
    // stereo to mono
    if (env->snk_channel < 2)
    {
        int16_t *p = (int16_t *)decode_buf;
        uint32_t samples = *out_len >> 2;
        for (uint32_t i = 0; i < samples; i++)
        {
            p[i] = p[i * 2 + env->snk_channel];
        }
        *out_len = *out_len >> 1;
    }

    return decode_buf;
}

static uint8_t *play_data_decode(a2dp_relay_sink_env_t *env, uint16_t *out_len)
{
    uint8_t *frm_ptr, *data, *media_pkt_ptr;
    uint16_t bytes_left;
    BTS2S_SBC_STREAM bss;
    uint8_t *ret = NULL;
    play_data_t *pt_data;


    pt_data = (play_data_t *)list_pop_front(&(env->playlist));
    env->pt_curdata = pt_data;

    *out_len = 0;
    data = (U8 *)pt_data;

    if (pt_data == NULL)
    {
        return ret;
    }

    if (pt_data->len <= (AV_FIXED_MEDIA_PKT_HDR_SIZE + 1))
    {
        //USER_TRACE("a2dp play decode len:%d\n", pt_data->len);
        free(pt_data);
        return ret;
    }

    //if (inst->snk_data.codec == AV_SBC)
    {
        frm_ptr = data + AV_FIXED_MEDIA_PKT_HDR_SIZE + 1;
        media_pkt_ptr = data + AV_FIXED_MEDIA_PKT_HDR_SIZE;
        bytes_left = pt_data->len - AV_FIXED_MEDIA_PKT_HDR_SIZE - 1;


        if (((*media_pkt_ptr) & 0x0f) > env->frms_per_payload)
        {
            env->frms_per_payload = (*media_pkt_ptr) & 0x0f;

            if (env->decode_buf)
            {
                free(env->decode_buf);
                env->decode_buf = NULL;
            }

            if (env->decode_buf == NULL)
            {
                U16 decode_buffer_size;
                U16 pcm_samples_per_sbc_frame;

                pcm_samples_per_sbc_frame = bts2_sbc_calculate_pcm_samples_per_sbc_frame(env->sbc_para.blocks, env->sbc_para.subbands);
                decode_buffer_size = pcm_samples_per_sbc_frame * env->frms_per_payload * 2;
                //USER_TRACE("frms_per_payload = %d,pcm_samples_per_sbc_frame = %d,decode_buffer_size = %d\n",
                //         frms_per_payload, pcm_samples_per_sbc_frame, decode_buffer_size);
                env->decode_buf = malloc(decode_buffer_size);
                env->decode_buf_len = decode_buffer_size;
            }
            RT_ASSERT(env->decode_buf);
        }

        bss.dst_len = env->decode_buf_len;
        bss.pdst = env->decode_buf;
        bss.src_len = bytes_left;
        bss.psrc = frm_ptr;

        bts2_sbc_decode(&bss);
        //LOG_I("dec(%x), %d, %x", frm_ptr, bytes_left, *frm_ptr);

        ret = env->decode_buf;
        *out_len = bss.dst_len_used;
        RT_ASSERT(bss.src_len_used == bss.src_len);
    }


    ret = play_data_decode_data_process(env, ret, out_len);
    free(pt_data);

    return ret;
}
extern void pll_freq_grade_set(uint8_t gr);

static int audio_bt_music_client_cb(audio_server_callback_cmt_t cmd, void *userdata, uint32_t unused)
{
    (void)userdata;
    (void)unused;
    if (cmd == as_callback_cmd_cache_empty || cmd == as_callback_cmd_cache_half_empty)
    {
        rt_event_send(g_playback_evt, PLAYBACK_GETDATA_EVENT_FLAG);
    }

    return 0;
}

static void decode_playback_thread(void *args)
{
    a2dp_relay_sink_env_t *env = a2dp_relay_sink_get_env();

    rt_uint32_t evt;
    play_data_t *pt_data;
    uint8_t *decode_data = NULL;
    uint16_t decode_len = 0;
    uint8_t  is_stopped = 1;
    uint8_t  debug_tx_cnt = 0;
    int  ret_write = 0;

    g_playback_evt = rt_event_create("playback_evt", RT_IPC_FLAG_FIFO);

    while (1)
    {
        evt = 0;
        rt_err_t err = rt_event_recv(g_playback_evt, PLAYBACK_GETDATA_EVENT_FLAG | PLAYBACK_START_EVENT_FLAG | PLAYBACK_STOPPING_EVENT_FLAG, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &evt);

        if (evt & PLAYBACK_STOPPING_EVENT_FLAG)
        {
            is_stopped = 1;
            rt_event_send(g_playback_evt, PLAYBACK_STOPPED_EVENT_FLAG);
            continue;
        }

        if (evt & PLAYBACK_START_EVENT_FLAG)
        {
            if (env->audio_client)
            {
                //USER_TRACE("bt_music: open again--\r\n");
                continue;
            }

            //decode_data = play_data_decode(env, &decode_len);
            //interval = decode_len * 1000 / BT_MUSIC_SAMPLERATE / 4; //use interval to rt_event_recv will crash rt_free
            //USER_TRACE("bt_music: open len=%d\r\n", decode_len);

            //USER_TRACE("decode src_len:%d, dst_len:%d\n", inst_data->snk_data.pt_curdata->len, decode_len);
#if 0
            if (decode_len == 0)
            {
                rt_thread_mdelay(5);
                rt_event_send(g_playback_evt, PLAYBACK_START_EVENT_FLAG);
                continue;
            }
#endif

            audio_parameter_t param = {0};
            //if (inst_data->snk_data.codec == AV_SBC)
            {
                param.write_samplerate = env->sbc_para.sample_freq;
            }

            param.write_channnel_num = env->chnl_nb;
            param.write_bits_per_sample = 16;
            param.write_cache_size = 8192;
            debug_tx_cnt = 0;
            env->audio_client = audio_open(AUDIO_TYPE_BT_MUSIC, AUDIO_TX, &param, audio_bt_music_client_cb, NULL);
            is_stopped = 0;
            BEGIN_ACCESS_BUF();
            env->relay_state = A2DP_RELAY_STATE_CODEC_OPENED;
            END_ACCESS_BUF();
        }
        if (evt & PLAYBACK_GETDATA_EVENT_FLAG)
        {
            if (debug_tx_cnt == 0)
            {
                //USER_TRACE("a2dp get data,total:%d,full:%d,empty:%d, curr %d\r\n", inst_data->snk_data.playlist.total_num,
                //           inst_data->snk_data.playlist.full_num, inst_data->snk_data.playlist.empty_num, inst_data->snk_data.playlist.cnt);
            }
            debug_tx_cnt++;

            if (is_stopped == 1 || env->relay_state != A2DP_RELAY_STATE_PLAYING || env->audio_client == NULL)
            {
                //USER_TRACE("snk: stop %d %d %x\r\n", is_stopped, inst_data->snk_data.play_state, inst_data->snk_data.audio_client);
                continue;
            }
        }

        if (env->relay_state != A2DP_RELAY_STATE_PLAYING)
            continue;

        while (env->extra_zero_byte)
        {
            if (env->extra_zero_byte <= env->decode_buf_len)
            {
                decode_len = env->extra_zero_byte;
                env->extra_zero_byte = 0;
            }
            else
            {
                decode_len = env->decode_buf_len;
                env->extra_zero_byte -= env->decode_buf_len;
            }
            decode_data = env->decode_buf;
            memset(decode_data, 0, decode_len);
            if (audio_write(env->audio_client, decode_data, decode_len) <= 0)
            {
                LOG_W("init zero byte failed");
                env->extra_zero_byte += decode_len;
                break;
            }
        }

        if (decode_len == 0)
        {
            LOG_I("buf is not enough");
            decode_data = play_data_decode(env, &decode_len);
        }


        while (decode_len > 0)
        {
            ret_write = audio_write(env->audio_client, decode_data, decode_len);

            if (ret_write < 0)
            {
                //USER_TRACE("playback write ret:%d\n", ret_write);
                break;
            }
            else if (ret_write == 0)
            {
                break;
            }
            else
            {
                if (!env->extra_zero_byte)
                    decode_data = play_data_decode(env, &decode_len);
            }
        }
    }
}

#define MAX_DELAY_THRESHOLD 60
#define MAX_DELAY_THRESHOLD_CNT 10
#define MIN_DELAY_STEP 20
#define MAX_TRACK_CNT 1000


static uint8_t pll_slow, pll_fast;
static uint8_t a2dp_relay_pll_reset(void)
{
    pll_slow = 0;
    pll_fast = 0;
    return 0;
}

static uint8_t a2dp_relay_pll_adjust(a2dp_relay_pll_ctrl_t *pll_ctrl, uint32_t local_delay, uint32_t peer_delay)
{
    static uint8_t pll_slow = 0, pll_fast = 0;
    uint8_t is_reset = 0;
    uint32_t abs_diff = peer_delay >= local_delay ? (peer_delay - local_delay) : (local_delay - peer_delay);
    int32_t diff = peer_delay - local_delay;
    int32_t diff_ave = 0;
    uint8_t is_add = 0, is_sub = 0;
    //static uint32_t track_cnt = 0;

    if (abs_diff > MAX_DELAY_THRESHOLD)
    {
        pll_ctrl->err_cnt++;
        if (pll_ctrl->err_cnt >= MAX_DELAY_THRESHOLD_CNT)
        {
            pll_ctrl->track_cnt = 0;
            is_reset = 1;
            pll_ctrl->err_cnt = 0;
        }
    }
    else
    {
        pll_ctrl->err_cnt = 0;
        pll_ctrl->track_cnt++;
        pll_ctrl->diff_total += diff;

        if (pll_ctrl->track_cnt >= MAX_TRACK_CNT)
        {
            diff_ave = pll_ctrl->diff_total / pll_ctrl->track_cnt;
            LOG_I("ave cnt %d diff %d", pll_ctrl->track_cnt, diff_ave);
            if ((diff_ave >= 30) && !pll_slow)
            {
                pll_slow = 1;
                pll_fast = 0;
                is_sub = 1;
            }
            else if ((diff_ave <= -30) && !pll_fast)
            {
                pll_fast = 1;
                pll_slow = 0;
                is_add = 1;
            }
            pll_ctrl->track_cnt = 0;
            pll_ctrl->diff_total = 0;
        }

        if (pll_slow > 0)
        {

        }

        if (is_add)
        {
            LOG_I("faster, slow_cnt %d, fast cnt %d", pll_slow, pll_fast);
            pll_freq_grade_set(4); //PLL_ADD_TEN_PPM
        }
        else if (is_sub)
        {
            LOG_I("slower, slow_cnt %d, fast cnt %d", pll_slow, pll_fast);
            pll_freq_grade_set(5); // PLL_SUB_HUND_PPM
        }
    }

    return is_reset;
}

static uint8_t a2dp_relay_data_handle(a2dp_relay_sink_env_t *env, uint8_t *data, uint16_t len)
{
    uint32_t left_byte = *((uint32_t *)data);
    static uint32_t count = 0;
    uint8_t left_list_cnt = data[4];
    play_data_t *pt_data = (play_data_t *)malloc(len);
    memcpy(pt_data, data, len);
    uint8_t list_cnt;
    uint8_t chnl_nb = env->chnl_nb;
    uint32_t intv_offload = PCM_DATA_CAL(RELAY_INTERVAL, env->sbc_para.sample_freq, chnl_nb);
    uint32_t freq = HAL_LPTIM_GetFreq();
    static uint32_t rd_cnt = 0;
    static uint32_t pre_rd_cnt = 0;

    pre_rd_cnt = rd_cnt;
    rd_cnt = HAL_HPAON_READ_GTIMER();

    pt_data->len = len;
    list_cnt = list_push_back(&env->playlist, &(pt_data->hdr));
    LOG_I("[%d]cur cnt %d, rem cnt %d, lb %d, freq:%d, dif_cnt %d", count++, list_cnt, left_list_cnt, left_byte, freq, rd_cnt - pre_rd_cnt);
    if (env->relay_state == A2DP_RELAY_STATE_CODEC_OPENED)
    {
        //USER_TRACE("av_snk.c open a2dp\r\n");
        if (env->relay_preparing == 0)
        {
            count = 0;
            uint8_t is_playing = 0;

            LOG_I("byte %d, cnt %d, compensate %d", left_byte, left_list_cnt, intv_offload);
            if (intv_offload < left_byte)
                left_byte -= intv_offload;
            else
                left_byte = 0;

            env->extra_zero_byte = left_byte;

            env->playlist.cnt_th = left_list_cnt + 1;

            if (list_cnt >= (left_list_cnt + 1))
            {
                list_pop_front_n(&env->playlist, list_cnt - left_list_cnt - 1);
                is_playing = 1;
            }

            if (is_playing == 1)
            {
                BEGIN_ACCESS_BUF();
                env->relay_state = A2DP_RELAY_STATE_PLAYING;
                END_ACCESS_BUF();
            }
            else
                env->relay_preparing = 1;
        }
        else
        {
            if (list_cnt > env->playlist.cnt_th)
            {
                BEGIN_ACCESS_BUF();
                env->relay_state = A2DP_RELAY_STATE_PLAYING;
                env->relay_preparing = 0;
                END_ACCESS_BUF();
            }
        }

    }

    if (env->relay_state == A2DP_RELAY_STATE_PLAYING)
    {
        uint8_t frm_per_payload = *(data + AV_FIXED_MEDIA_PKT_HDR_SIZE) & 0xF;
        uint32_t local_left_ms;
        audio_ioctl(env->audio_client, 1, &local_left_ms);
        uint32_t cnt_size = frm_per_payload * bts2_sbc_calculate_pcm_samples_per_sbc_frame(env->sbc_para.blocks, env->sbc_para.subbands) * chnl_nb;
        uint32_t cnt_ms = PCM_TIME_CAL(cnt_size, env->sbc_para.sample_freq, chnl_nb);
        uint32_t local_delay = list_cnt * cnt_ms + local_left_ms;
        uint32_t peer_delay = (left_list_cnt + 1) * cnt_ms + PCM_TIME_CAL(left_byte, env->sbc_para.sample_freq, chnl_nb) - RELAY_INTERVAL;

        LOG_I("cnt_ms %d(%d), loc %d, peer %d, diff %d", cnt_ms, local_left_ms, local_delay, peer_delay, peer_delay - local_delay);
#if 1
        if (a2dp_relay_pll_adjust(&env->pll_ctrl, local_delay, peer_delay) == 1)
        {
            list_all_free(&env->playlist);
            BEGIN_ACCESS_BUF();
            env->relay_state = A2DP_RELAY_STATE_CODEC_OPENED;
            env->relay_preparing = 0;
            END_ACCESS_BUF();
        }
#endif
        rt_event_send(g_playback_evt, PLAYBACK_GETDATA_EVENT_FLAG);
    }

    if (list_cnt == 0) // full
    {
        list_hdr_t *hdr;
        hdr = list_pop_front(&env->playlist);
        free(hdr);
    }
    return 0;
}

static uint8_t a2dp_relay_stop_handler(a2dp_relay_sink_env_t *env)
{
    rt_uint32_t evt;

    LOG_I("stop_audio_playback state:%d\n", env->relay_state);
    if (env->relay_state == A2DP_RELAY_STATE_PLAYING)
    {
        rt_event_send(g_playback_evt, PLAYBACK_STOPPING_EVENT_FLAG);
        rt_event_recv(g_playback_evt, PLAYBACK_STOPPED_EVENT_FLAG, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &evt);
#if defined(AUDIO_USING_MANAGER) && defined(AUDIO_BT_AUDIO)
        audio_close(env->audio_client);
        env->audio_client = NULL;
#endif
        env->relay_state = A2DP_RELAY_STATE_CONN;
        env->relay_preparing = 0;
    }
    a2dp_relay_pll_reset();
    list_all_free(&(env->playlist));

    if (env->decode_buf)
    {
        // frm payload could be updated before decode
        env->frms_per_payload = 0;
        bfree(env->decode_buf);
        env->decode_buf = NULL;
    }

    return 0;
}



static uint8_t a2dp_relay_command_parse(a2dp_relay_sink_env_t *env, a2dp_relay_data_t *data, uint16_t len)
{
    switch (data->type)
    {
    case A2DP_RELAY_START:
    {
        if (env->role == A2DP_RELAY_SINK)
        {
            uint8_t *idata = (uint8_t *)&data->data;
            env->relay_state = A2DP_RELAY_STATE_PLAYING;
            env->codec = idata[0];
            if (env->codec == CODEC_SBC)
            {
                env->sbc_para.chnl_mode = idata[1];
                env->sbc_para.alloc_method = idata[2];
                env->sbc_para.sample_freq = (uint16_t)(idata[4] << 8) | idata[3];
                env->sbc_para.blocks = idata[5];
                env->sbc_para.subbands = idata[6];
                env->sbc_para.bitpool = idata[7];
                env->playlist.cnt_th = idata[8];
                env->max_playing_cnt = idata[9];
                env->snk_channel = idata[10];
                env->chnl_nb = env->snk_channel < 2 ? 1 : 2;
                LOG_I("snk channel %d", env->snk_channel);


                uint8_t res = bts2_sbc_decode_cfg(env->sbc_para.chnl_mode,
                                                  env->sbc_para.alloc_method,
                                                  env->sbc_para.sample_freq,
                                                  env->sbc_para.blocks,
                                                  env->sbc_para.subbands,
                                                  env->sbc_para.bitpool);

                RT_ASSERT(res != 0);

                uint16_t sbc_frame_size;
                uint16_t decode_buffer_size;
                uint16_t pcm_samples_per_sbc_frame;

                sbc_frame_size = bts2_sbc_calculate_framelen(env->sbc_para.chnl_mode,
                                 env->sbc_para.blocks,
                                 env->sbc_para.subbands,
                                 env->sbc_para.bitpool);
                env->frms_per_payload = (U16)((1005 - 14) / sbc_frame_size + 1);
                pcm_samples_per_sbc_frame = bts2_sbc_calculate_pcm_samples_per_sbc_frame(env->sbc_para.blocks, env->sbc_para.subbands);
                decode_buffer_size = pcm_samples_per_sbc_frame * env->frms_per_payload * 2;
                //USER_TRACE("sbc_frame_size = %d,pcm_samples_per_sbc_frame = %d,decode_buffer_size = %d\n",
                //          sbc_frame_size, pcm_samples_per_sbc_frame, decode_buffer_size);
                if (env->decode_buf == NULL)
                {
                    env->decode_buf = malloc(decode_buffer_size);
                    env->decode_buf_len = decode_buffer_size;
                }

                rt_event_send(g_playback_evt, PLAYBACK_START_EVENT_FLAG);

            }
        }

    }
    break;
    case A2DP_RELAY_DATA:
    {
        if (env->role == A2DP_RELAY_SINK)
        {
            a2dp_relay_data_handle(env, (uint8_t *)data->data, data->len);
        }
    }
    break;
    case A2DP_RELAY_STOP:
    {
        if (env->role == A2DP_RELAY_SINK)
        {
            a2dp_relay_stop_handler(env);
        }
    }
    break;
    case A2DP_RELAY_VOL:
    {
        uint8_t volume = data->data[0];
        LOG_I("vol changed %d", volume);
#ifdef AUDIO_USING_MANAGER
        uint8_t local_vol = bt_interface_avrcp_abs_vol_2_local_vol(volume, audio_server_get_max_volume());
        audio_server_set_private_volume(AUDIO_TYPE_BT_MUSIC, local_vol);
#endif
    }
    break;
    default:
        break;
    }

    return 0;
}

void a2dp_data_snk_handler(ble_serial_tran_event_t event, uint8_t *data)
{
    a2dp_relay_sink_env_t *env = a2dp_relay_sink_get_env();
    if (env->role != A2DP_RELAY_SINK)
        return;

    switch (event)
    {
    case BLE_SERIAL_TRAN_OPEN:
    {
        ble_serial_open_t *open = (ble_serial_open_t *)data;

        // only set for 1st device
        if (env->handle == A2DP_RELAY_INVALID_HDL)
        {
            LOG_I("data opened");
            uint8_t role = A2DP_RELAY_SINK;
            env->handle = open->handle;
            env->relay_state = A2DP_RELAY_STATE_CONN;

            if (env->callback)
                env->callback(A2DP_RELAY_EVT_PAIRED, &role, 1);
        }
        break;
    }
    case BLE_SERIAL_TRAN_DATA:
    {

        ble_serial_tran_data_t *t_data = (ble_serial_tran_data_t *)data;
        if (env->handle == t_data->handle && t_data->cate_id == A2DP_RELAY_ID)
        {
            a2dp_relay_command_parse(env, (a2dp_relay_data_t *)t_data->data, t_data->len);
        }
    }
    break;
    case BLE_SERIAL_TRAN_CLOSE:
    {
        ble_serial_close_t *close = (ble_serial_close_t *)data;
        LOG_I("chan closed %d", close->handle);
        if (env->handle == close->handle)
        {
            a2dp_relay_stop_handler(env);
            env->handle = A2DP_RELAY_INVALID_HDL;
            env->relay_state = A2DP_RELAY_STATE_IDLE;
        }

        if (env->callback)
            env->callback(A2DP_RELAY_EVT_DISABLED, NULL, 0);
    }
    break;
    case BLE_SERIAL_TRAN_ERROR:
    {
        ble_serial_tran_error_t *err = (ble_serial_tran_error_t *)data;
        LOG_E("snk tran err %d", err->error);
    }
    break;
    default:
        break;
    }

}
BLE_SERIAL_TRAN_EXPORT(A2DP_RELAY_ID, a2dp_data_snk_handler);


SIBLES_ADVERTISING_CONTEXT_DECLAR(g_app_advertising_context);

static uint8_t ble_app_advertising_event(uint8_t event, void *context, void *data)
{
    a2dp_relay_sink_env_t *env = a2dp_relay_sink_get_env();

    switch (event)
    {
    case SIBLES_ADV_EVT_ADV_STARTED:
    {
        sibles_adv_evt_startted_t *evt = (sibles_adv_evt_startted_t *)data;
        if (evt->status != HL_ERR_NO_ERROR)
            env->is_advertising = 0;

        if (env->callback)
        {
            uint8_t result = env->is_advertising ? A2DP_RELAY_ERR_NO_ERROR : A2DP_RELAY_ERR_GENERAL;
            env->callback(A2DP_RELAY_EVT_ENABLED, &result, 1);
        }
        LOG_I("ADV start resutl %d, mode %d\r\n", evt->status, evt->adv_mode);
        break;
    }
    case SIBLES_ADV_EVT_ADV_STOPPED:
    {
        sibles_adv_evt_stopped_t *evt = (sibles_adv_evt_stopped_t *)data;
        env->is_advertising = 0;
        if (env->relay_state == A2DP_RELAY_STATE_IDLE)
        {
            if (env->callback)
            {
                env->callback(A2DP_RELAY_EVT_DISABLED, NULL, 0);
            }
        }
        LOG_I("ADV stopped reason %d, mode %d\r\n", evt->reason, evt->adv_mode);
        break;
    }
    default:
        break;
    }
    return 0;
}

#define DEFAULT_LOCAL_NAME "RELAY"
/* Enable advertise via advertising service. */
static void ble_app_advertising_init(void)
{
    sibles_advertising_para_t para = {0};
    uint8_t svc_uuid[ATT_UUID_128_LEN] = A2DP_RELAY_SVC_UUID;
    uint8_t ret;

    char local_name[31] = {0};
    uint8_t manu_additnal_data[] = {0x20, 0xC4, 0x00, 0x91};
    uint16_t manu_company_id = SIG_SIFLI_COMPANY_ID;

    memcpy(local_name, DEFAULT_LOCAL_NAME, sizeof(DEFAULT_LOCAL_NAME));

    ble_gap_dev_name_t *dev_name = malloc(sizeof(ble_gap_dev_name_t) + strlen(local_name));
    dev_name->len = strlen(local_name);
    memcpy(dev_name->name, local_name, dev_name->len);
    ble_gap_set_dev_name(dev_name);
    free(dev_name);

    para.own_addr_type = GAPM_STATIC_ADDR;
    para.config.adv_mode = SIBLES_ADV_CONNECT_MODE;
    /* Keep advertising till disable it or connected. */
    para.config.mode_config.conn_config.duration = 0x0;
    para.config.mode_config.conn_config.interval = 0x30;
    para.config.max_tx_pwr = 0x7F;
    /* Advertising will re-start after disconnected. */
    para.config.is_auto_restart = 0;
    /* Scan rsp data is same as advertising data. */
    //para.config.is_rsp_data_duplicate = 1;

    /* Prepare name filed. Due to name is too long to put adv data, put it to rsp data.*/
    para.rsp_data.completed_name = rt_malloc(rt_strlen(local_name) + sizeof(sibles_adv_type_name_t));
    para.rsp_data.completed_name->name_len = rt_strlen(local_name);
    rt_memcpy(para.rsp_data.completed_name->name, local_name, para.rsp_data.completed_name->name_len);

    /* Prepare manufacturer filed .*/
    para.adv_data.completed_uuid = rt_malloc(sizeof(sibles_adv_type_srv_uuid_t) + sizeof(sibles_adv_uuid_t));
    para.adv_data.completed_uuid->count = 1;
    para.adv_data.completed_uuid->uuid_list[0].uuid_len = ATT_UUID_128_LEN;
    memcpy(para.adv_data.completed_uuid->uuid_list[0].uuid.uuid_128, svc_uuid, ATT_UUID_128_LEN);

    para.evt_handler = ble_app_advertising_event;

    ret = sibles_advertising_init(g_app_advertising_context, &para);
    if (ret == SIBLES_ADV_NO_ERR)
    {
        LOG_E("adv init failed");

    }

    rt_free(para.rsp_data.completed_name);
    rt_free(para.adv_data.completed_uuid);
}



uint8_t a2dp_relay_sink_enable(void)
{
    uint8_t ret = A2DP_RELAY_ERR_GENERAL;
    a2dp_relay_sink_env_t *env = a2dp_relay_sink_get_env();
    do
    {
        if (env->relay_state >= A2DP_RELAY_STATE_CONN)
        {
            ret = A2DP_RELAY_ERR_ALREADY_CONN;
            break;
        }

        if (env->is_advertising)
        {
            ret = A2DP_RELAY_ERR_UNDER_RUN;
            break;
        }

        sibles_advertising_start(g_app_advertising_context);
        env->role = A2DP_RELAY_SINK;
        ret = A2DP_RELAY_ERR_NO_ERROR;

    }
    while (0);

    return ret;
}

uint8_t a2dp_relay_sink_disable(void)
{
    uint8_t ret = A2DP_RELAY_ERR_NO_ERROR;
    a2dp_relay_sink_env_t *env = a2dp_relay_sink_get_env();
    do
    {
        if (env->relay_state == A2DP_RELAY_STATE_IDLE &&
                env->is_advertising == 1)
        {
            sibles_advertising_stop(g_app_advertising_context);
            break;
        }

        if (env->relay_state >= A2DP_RELAY_STATE_CONN)
        {
            ble_gap_disconnect_t dis =
            {
                .conn_idx = env->handle,
                .reason = CO_ERROR_REMOTE_USER_TERM_CON,
            };
            ble_gap_disconnect(&dis);
            break;
        }

        ret = A2DP_RELAY_ERR_INVALID;
    }
    while (0);

    return ret;
}

void a2dp_relay_sink_init(a2dp_relay_callback callback)
{
    a2dp_relay_sink_env_t *env = a2dp_relay_sink_get_env();
    env->buf_sem = rt_sem_create("le_avrly", 1, RT_IPC_FLAG_PRIO);
    env->start_mux = rt_mutex_create("le_avrly", RT_IPC_FLAG_FIFO);
    env->role = A2DP_RELAY_NO_ROLE;
    env->handle = A2DP_RELAY_INVALID_HDL;
    env->playlist.cnt = 0;
    env->playlist.cnt_th = 5;
    env->playlist.first = NULL;
    env->playlist.last = NULL;
    env->callback = callback;
    ble_serial_tran_init();
    ble_app_advertising_init();
    rt_thread_t tid = rt_thread_create("le_decode", decode_playback_thread, NULL, DEPLAYBACK_STACK_SIZE, RT_THREAD_PRIORITY_HIGH, RT_THREAD_TICK_DEFAULT);
    if (tid != NULL)
        rt_thread_startup(tid);
}

#endif // A2DP_RELAY_SERVICE

