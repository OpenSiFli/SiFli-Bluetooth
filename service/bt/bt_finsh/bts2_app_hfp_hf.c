/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bts2_app_inc.h"
#include "rtthread.h"
#include "bf0_sibles.h"

#ifdef RT_USING_BT
    #include "bt_rt_device.h"
#endif

extern bts2_app_stru *bts2g_app_p;

#ifdef CFG_HFP_HF

#define LOG_TAG         "btapp_hf"
#include "log.h"

#if defined(AUDIO_USING_MANAGER) && !defined(BT_USING_HF)
    #include "hfp_audio_api.h"
#endif

#define HFP_HF_LOCAL_FEATURES        (  HFP_HF_FEAT_ECNR  | \
                                        HFP_HF_FEAT_3WAY  | \
                                        HFP_HF_FEAT_CLI   | \
                                        HFP_HF_FEAT_VREC  | \
                                        HFP_HF_FEAT_VOL   | \
                                        HFP_HF_FEAT_ECS   | \
                                        HFP_HF_FEAT_ECC   | \
                                        HFP_HF_FEAT_CODEC | \
                                        HFP_HF_FEAT_ESCO  )

// sco handle: Packet_Status_Flag inside if any.
// static void hfp_hf_audio_cb_fn(U16 sco_hdl, U8 sco_len, U8 *data)
// {
// }
/*******************************************device info func start**********************************************/
static void bt_hfp_hf_app_init_device_info(bts2_hfp_hf_inst_data *hf_data)
{
    if (hf_data)
    {
        for (int i = 0; i < CFG_MAX_HFP_CONN_NUM; i++)
        {
            bmemset(&hf_data->devices_info[i], 0x00, sizeof(bts2_hfp_hf_device_info));
            hf_data->devices_info[i].is_use = 1;
            hf_data->devices_info[i].mux_id = 0xff;
            hf_data->devices_info[i].pre_profile_state = HFP_DEVICE_DISCONNECTED;
            hf_data->devices_info[i].profile_state = HFP_DEVICE_DISCONNECTED;
            hf_data->devices_info[i].srv_chnl = 0xff;
            hf_data->devices_info[i].voice_flag = 0;
            hf_data->devices_info[i].profile_type = HF_CONN;
            hf_data->devices_info[i].sco_hdl = 0xffff;
            hf_data->devices_info[i].peer_features = 0x0000;
            bmemset(&hf_data->devices_info[i].cind_status, 0x00, sizeof(bts2_hfp_hf_cind));
        }
    }
}

static bts2_hfp_hf_device_info *bt_hfp_hf_app_get_device_by_mux_id(bts2_hfp_hf_inst_data *hf_data, U8 mux_id)
{
    if (hf_data)
    {
        for (int i = 0; i < CFG_MAX_HFP_CONN_NUM; i++)
        {
            if (hf_data->devices_info[i].mux_id == mux_id)
            {
                return &hf_data->devices_info[i];
            }
        }
    }
    return NULL;
}

static bts2_hfp_hf_device_info *bt_hfp_hf_app_get_device_by_bd_addr(bts2_hfp_hf_inst_data *hf_data, BTS2S_BD_ADDR *bd)
{
    if (hf_data)
    {
        for (int i = 0; i < CFG_MAX_HFP_CONN_NUM; i++)
        {
            if (bd_eq(bd, &(hf_data->devices_info[i].hfp_bd)))
            {
                return &hf_data->devices_info[i];
            }
        }
    }
    return NULL;
}

static bts2_hfp_hf_device_info *bt_hfp_hf_app_alloc_device(bts2_hfp_hf_inst_data *hf_data)
{
    if (hf_data)
    {
        for (int i = 0; i < CFG_MAX_HFP_CONN_NUM; i++)
        {
            if (hf_data->devices_info[i].is_use)
            {
                hf_data->devices_info[i].is_use = 0;
                return &hf_data->devices_info[i];
            }
        }
    }
    return NULL;
}

static void bt_hfp_hf_app_dealloc_device(bts2_hfp_hf_device_info *device)
{
    if (device)
    {
        bmemset(device, 0x00, sizeof(bts2_hfp_hf_device_info));
        device->is_use = 1;
        device->mux_id = 0xff;
        device->pre_profile_state = HFP_DEVICE_DISCONNECTED;
        device->profile_state = HFP_DEVICE_DISCONNECTED;
        device->srv_chnl = 0xff;
        device->voice_flag = 0;
        device->profile_type = HF_CONN;
        device->sco_hdl = 0xffff;
        device->peer_features = 0x0000;
        bmemset(&device->cind_status, 0x00, sizeof(bts2_hfp_hf_cind));
    }
}

static bts2_hfp_hf_device_info *bt_hfp_hf_app_get_busy_device(bts2_hfp_hf_inst_data *hf_data)
{
    if (hf_data)
    {
        for (int i = 0; i < CFG_MAX_HFP_CONN_NUM; i++)
        {
            if (hf_data->devices_info[i].is_use == 0)
            {
                return &hf_data->devices_info[i];
            }
        }
    }
    return NULL;
}
/*******************************************device info func end************************************************/

static bts2_hfp_hf_inst_data *bt_hfp_hf_get_context()
{
    bts2_app_stru *bts2_app_data = getApp();
    return bts2_app_data->hfp_hf_ptr;
}

static U8 bt_hfp_is_support_feature(bts2_hfp_hf_device_info *device_info, U16 feature)
{
    bts2_hfp_hf_inst_data *hfp_context = bt_hfp_hf_get_context();
    if (device_info && (device_info->peer_features & feature))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void bt_hfp_hf_app_service_state_update(bts2_hfp_st new_state)
{
    bts2_hfp_hf_inst_data *hfp_context = bt_hfp_hf_get_context();
    USER_TRACE("hfp hf profile service new_date:%d, old_state:%d", new_state, hfp_context->st);
    hfp_context->st = new_state;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_hfp_hf_init(bts2_app_stru *bts2_app_data)
{
    bts2_app_data->hfp_hf_ptr = &bts2_app_data->hfp_hf_inst;
    bt_hfp_hf_app_service_state_update(hfp_idle);
    bt_hfp_hf_app_init_device_info(bts2_app_data->hfp_hf_ptr);
#if defined(AUDIO_USING_MANAGER) && !defined(BT_USING_HF)
    hfp_audio_init();
#endif

}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_start_enb(bts2_app_stru *bts2_app_data)
{
    bts2_hfp_hf_inst_data *hfp_context = bt_hfp_hf_get_context();
    bt_err_t ret = BT_ERROR_STATE;

    switch (hfp_context->st)
    {
    case hfp_idle:
    case hfp_disb:
    {
        bt_hfp_hf_app_service_state_update(hfp_enbd);
        hfp_hf_register(HFP_HF_LOCAL_FEATURES);
        ret = BT_EOK;
        break;
    }
    default:
    {
        if (hfp_context->st != hfp_enbd)
        {
            USER_TRACE(">> Hf enable fail\n");
        }
        break;
    }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_start_disb(bts2_app_stru *bts2_app_data)
{
    bts2_hfp_hf_inst_data *hfp_context = bt_hfp_hf_get_context();
    bt_err_t ret = BT_ERROR_STATE;

    switch (hfp_context->st)
    {
    case hfp_enbd:
    {
        hfp_hf_deregister();
        bt_hfp_hf_app_service_state_update(hfp_disb);
        ret = BT_EOK;
        USER_TRACE(">> Hf disable\n");
        break;
    }
    default:
    {
        USER_TRACE(">> Hf disable failed\n");
        break;
    }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_connect_request(BTS2S_BD_ADDR *bd)
{
    bts2_hfp_hf_inst_data *hfp_context = bt_hfp_hf_get_context();
    bt_err_t ret = BT_ERROR_STATE;

    if (bd_is_empty(bd))
    {
        return BT_ERROR_INPARAM;
    }

    switch (hfp_context->st)
    {
    case hfp_enbd:
    {
        hfp_hf_connect(bd, HF_CONN);
        ret = BT_EOK;
        break;
    }
    default:
    {
        break;
    }
    }
    USER_TRACE("bt_hfp_hf_connect_request 0x%2x\n", ret);
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_start_disc(BTS2S_BD_ADDR *bd)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bt_err_t ret = BT_ERROR_STATE;

    if (bd_is_empty(bd))
    {
        return BT_ERROR_INPARAM;
    }

    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_bd_addr(hf_data, bd);
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_disconnect(bd, HF_CONN);
            ret = BT_EOK;
            USER_TRACE(">> Hf disconnect send\n");
            break;
        }
        default:
        {
            USER_TRACE(">> Hf disconnect fail");
            break;
        }
        }
    }

    return ret;
}

#if 1 //!defined(BT_CONNECT_SUPPORT_MULTI_LINK)
uint8_t g_mux_id = 0;
U8 bt_hfp_hf_get_ring_type(void)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    return bt_hfp_is_support_feature(device_info, HFP_AG_FEAT_INBAND);
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_audio_transfer(U8 type)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bt_err_t ret = BT_ERROR_STATE;
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            // type 0:connect audio type 1 :disconnect audio
            if (type == 0)
            {
                hfp_hf_connect_audio(&device_info->hfp_bd, 0);//connect audio
                ret = BT_EOK;
            }
            else if (type == 1)
            {
                hfp_hf_disconnect_audio(&device_info->hfp_bd, 0);//disconect audio
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
        {
            USER_TRACE(">> Transfer audio path fail\n");
            break;
        }
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_voice_recog_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_voice_recog_send(U8 active)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (active == 0 || active == 1)
            {
                hfp_hf_send_at_bvra_api(g_mux_id, HF_CONN, active);
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_dial_by_mem_send(U16 memory)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            char data[6];
            int at_len = 0;
            at_len = snprintf(data, sizeof(data), ">%u;", memory);
            hfp_hf_send_at_atd_api(g_mux_id, HF_CONN, (U8 *)data, (U8) at_len);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_last_num_dial_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_last_num_dial_send(void)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_bldn_api(g_mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }

    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_make_call_by_number_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_make_call_by_number_send(U8 *payload, U8 payload_len)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            char *data;
            int p_payload_len = payload_len + 1;
            data = (char *)bmalloc(p_payload_len);
            if (data)
            {
                bmemcpy(data, payload, payload_len);
                data[payload_len] = ';';
                hfp_hf_send_at_atd_api(g_mux_id, HF_CONN, (U8 *)data, (U8) p_payload_len);
                USER_TRACE("data %s len %d input_len %d", data, p_payload_len, payload_len);
                ret = BT_EOK;
                bfree(data);
            }
            else
                ret = BT_ERROR_OUT_OF_MEMORY;
            break;
        }
        default:
        {
            USER_TRACE(">> dial out fail\n");
            break;
        }
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Start_hf_answer_req_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_answer_call_send(void)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_ata_api(g_mux_id, HF_CONN);
            USER_TRACE(">> Answer the incoming call\n");
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_hangup_call_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_hangup_call_send(void)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_chup_api(g_mux_id, HF_CONN);
            USER_TRACE(">> hfp_hf reject terminal the call\n");
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_update_spk_vol
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_update_spk_vol(U8 vol)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (0 <= vol && vol <= 15)
            {
                hfp_hf_send_at_vgs_api(g_mux_id, HF_CONN, (U8)vol); //just send 0---15
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_update_mic_vol
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_update_mic_vol(U8 vol)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (0 <= vol && vol <= 15)
            {
                hfp_hf_send_at_vgm_api(g_mux_id, HF_CONN, vol); //just send 0---15
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_at_btrh_query_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_btrh_query_send(void)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_btrh_api(g_mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_at_btrh_cmd_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_btrh_cmd_send(U8 mode)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    USER_TRACE(">> enter \n");
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_btrh_mode_api(g_mux_id, HF_CONN, mode);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_at_binp_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_binp_send(void)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            //Attach a Phone Number to a Voice Tag.
            hfp_hf_send_at_binp_api(g_mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_at_clip_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_clip_send(U8 enable)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            //Enable calling Line Identification (CLI) Notification.
            if (enable == 0 || enable == 1)
            {
                hfp_hf_send_at_clip_api(g_mux_id, HF_CONN, enable);
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_at_cmee_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_cmee_send(BOOL val)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (val == 0 || val == 1)
            {
                hfp_hf_send_at_cmee_api(g_mux_id, HF_CONN, val);
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Start_at_cnum_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_cnum_send(void)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_cnum_api(g_mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_at_ccwa_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_ccwa_send(BOOL val)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (val == 0 || val == 1)
            {
                hfp_hf_send_at_ccwa_api(g_mux_id, HF_CONN, val); //active
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_at_chld_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_chld_send(U8 *payload, U8 payload_len)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_chld_control_api(g_mux_id, HF_CONN, payload, payload_len);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_at_clcc_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_clcc_send(void)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_clcc_api(g_mux_id, HF_CONN);
            // ok
            //during a call process, solution can send clcc the get info. so ptr->st maybe hfp_conned or hfp_calling
            USER_TRACE(">> List current call status\n");
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_cops_cmd_send(void)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info && (device_info->profile_state == HFP_DEVICE_CONNECTED))
    {
        //hfp_hf_copp_srv_req(COPSMODE, COPSFMTE);
        char *payload =  "3,0";
        U8 payload_len = strlen(payload);
        hfp_hf_send_at_cops_cmd_api(g_mux_id, HF_CONN, (U8 *)payload, payload_len);
        ret = BT_EOK;
        USER_TRACE(">> set the cops information\n");
    }
    else
    {
        USER_TRACE(">> Not in connected state\n");
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_at_dtmf_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_dtmf_send(char key)
{
    //U8 *data;
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_vts_api(g_mux_id, HF_CONN, key);
            ret = BT_EOK;
            break;
        }
        default:
            USER_TRACE("-- Hf state error, current state is %d\n", device_info->profile_state);
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Start_ccwa_req_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_at_nrec_send(void)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            //The HF may disable the echo canceling and noise reduction functions resident in the AG via the AT+NREC command.
            hfp_hf_send_at_nrec_api(g_mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_update_batt_send
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bt_err_t bt_hfp_hf_update_batt_send(U8 batt_val)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (0 <= batt_val && batt_val <= 9)
            {
                char data[8];
                int at_len = 0;
                at_len = snprintf(data, sizeof(data), "1,1,%d", batt_val);
                hfp_hf_send_at_batt_update_api(g_mux_id, HF_CONN, (U8 *)data, (U8) at_len); //just 0~9
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t hfp_hf_get_at_cind_status()
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, g_mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_cind_status_api(g_mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}
#endif

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
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
void bt_hfp_hf_rfc_conn_accept_hdl(BTS2S_BD_ADDR *bd, U8 srv_chnl)
{
    hfp_hf_connect_ind_res(bd, srv_chnl, TRUE);
}

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
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
void bt_hfp_hf_rfc_conn_rej_hdl(BTS2S_BD_ADDR *bd, U8 srv_chnl)
{
    hfp_hf_connect_ind_res(bd, srv_chnl, FALSE);
}

static void bt_hfp_hf_at_cmd_cfm_hdl(BTS2S_HF_AT_CMD_CFM *msg)
{
    USER_TRACE("bt_hfp_at_cmd_cfm cmd_id:0x%2x  res:0x%2x", msg->at_cmd_id, msg->res);

    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, msg->mux_id);

    switch (msg->at_cmd_id)
    {
    case HFP_HF_AT_BVRA:
    case HFP_HF_AT_ATD:
    case HFP_HF_AT_BLDN:
    {
        break;
    }
    case HFP_HF_AT_CLCC:
    {
        break;
    }
    case HFP_HF_AT_VTS:
    case HFP_HF_AT_VGS:
    {
        break;
    }
    case HFP_HF_AT_BCC:
    case HFP_HF_AT_CIND_STATUS:
    case HFP_HF_AT_CMER:
    case HFP_HF_AT_CHLD_CMD:
    case HFP_HF_AT_CMEE:
    case HFP_HF_AT_BIA:
    case HFP_HF_AT_CLIP:
    case HFP_HF_AT_CCWA:
    case HFP_HF_AT_COPS_CMD:
    case HFP_HF_AT_VGM:
    case HFP_HF_AT_ATA:
    case HFP_HF_AT_CHUP:
    case HFP_HF_AT_BTRH:
    case HFP_HF_AT_BTRH_MODE:
    case HFP_HF_AT_CNUM:
    case HFP_HF_AT_NREC:
    case HFP_HF_AT_BINP:
    {
        break;
    }

    default:
        break;
    }

    bt_notify_at_cmd_cfm_t at_cmd_cfm;
    at_cmd_cfm.at_cmd_id = msg->at_cmd_id;
    at_cmd_cfm.profile_channel = msg->mux_id;
    at_cmd_cfm.res = msg->res;
    bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_AT_CMD_CFM, &at_cmd_cfm, sizeof(bt_notify_at_cmd_cfm_t));
}

U8 bt_hfp_hf_get_ciev_info(BTS2S_HF_CIEV_IND *msg)
{
    int ptr;
    char *str1 = "call";
    char *str2 = "callheld";
    char *str3 = "callsetup";
    char *str4 = "service";
    char *str5 = "signal";
    char *str6 = "roam";
    char *str7 = "battchg";
    bt_notify_cind_ind_t ind = {0};
    bts2_hfp_hf_inst_data *inst_data = bt_hfp_hf_get_context();

    ptr = strcmp(msg->name, str1);//call
    if (0 == ptr)
    {
        ind.type = HFP_AG_CIND_CALL_TYPE;
        ind.val = msg->val;
        USER_TRACE(">>ciev call st %d callStatus %d\n", inst_data->st, msg->val);
    }

    ptr = strcmp(msg->name, str2);//callheld
    if (0 == ptr)
    {
        ind.type = HFP_AG_CIND_CALLHELD_TYPE;
        ind.val = msg->val;
        USER_TRACE(">>ciev call hold st %d callStatus%d\n", inst_data->st, msg->val);
    }

    ptr = strcmp(msg->name, str3);//callsetup
    if (0 == ptr)
    {
        ind.type = HFP_AG_CIND_CALLSETUP_TYPE;
        ind.val = msg->val;
        USER_TRACE(">>ciev call setup callStatus %d\n", msg->val);
    }

    ptr = strcmp(msg->name, str4);//service

    if (0 == ptr)
    {
        ind.type = HFP_AG_CIND_SERVICE_TYPE;
        ind.val = msg->val;
    }

    ptr = strcmp(msg->name, str5);//signal

    if (0 == ptr)
    {
        ind.type = HFP_AG_CIND_SIGNAL_TYPE;
        ind.val = msg->val;
    }

    ptr = strcmp(msg->name, str6);//roam

    if (0 == ptr)
    {
        ind.type = HFP_AG_CIND_ROAM_TYPE;
        ind.val = msg->val;
    }

    ptr = strcmp(msg->name, str7);//battchg

    if (0 == ptr)
    {
        ind.type = HFP_AG_CIND_BATT_TYPE;
        ind.val = msg->val;
    }

    USER_TRACE("CIEV name:%s val:%d", msg->name, msg->val);
    ind.profile_channel = msg->mux_id;
    bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_INDICATOR_UPDATE,
                                 &ind, sizeof(bt_notify_cind_ind_t));
    return msg->val;
}


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      bt_hfp_hf_msg_hdl
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_hfp_hf_msg_hdl(bts2_app_stru *bts2_app_data)
{
    U16 *msg_type;
    bts2_hfp_hf_inst_data *inst_data;
    inst_data = bts2_app_data->hfp_hf_ptr;

    msg_type = (U16 *)bts2_app_data->recv_msg;
    USER_TRACE("hfp msg *msg_type %x inst_data->st %d\n", *msg_type, inst_data->st);

    switch (*msg_type)
    {
    case BTS2MU_HF_UNREG_CFM:
    {
        BTS2S_HF_DISB_CFM *msg;
        msg = (BTS2S_HF_DISB_CFM *)bts2_app_data->recv_msg;
        break;
    }
    case BTS2MU_HF_REG_CFM:
    {
        BTS2S_HF_ENB_CFM *msg;
        msg = (BTS2S_HF_ENB_CFM *)bts2_app_data->recv_msg;
        if (msg->profile_type == HF_CONN && msg->res == BTS2_SUCC)
        {
            USER_TRACE(">> Handfree enable success\n");
        }

        if (msg->profile_type == HS_CONN && msg->res == BTS2_SUCC)
        {
            USER_TRACE(">> Headset enable success\n");
        }
        break;
    }
    case BTS2MU_HF_CONN_IND:
    {
        BTS2S_HF_CONN_IND *msg;
        msg = (BTS2S_HF_CONN_IND *)bts2_app_data->recv_msg;
        hfp_hf_connect_ind_res(&msg->bd, msg->srv_chnl, TRUE);

        //20220727:add last_conn_bd
        bts2_app_data->last_conn_bd = msg->bd;
#ifdef BTS2_APP_MENU
        bts2_app_data->menu_id = menu_hfp_hf;
        bt_disply_menu(bts2_app_data);
#endif
        break;
    }
    case BTS2MU_HF_CONN_CFM:
    {
        BTS2S_HF_CONN_CFM *msg;
        msg = (BTS2S_HF_CONN_CFM *)bts2_app_data->recv_msg;
        USER_TRACE("BTS2MU_HF_CONN_CFM state %d res:%x", msg->device_state, msg->res);
        if ((msg->res == BTS2_SUCC) && (msg->device_state == HFP_DEVICE_CONNECTED))
        {
            bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(inst_data, msg->mux_id);
            if (!device_info)
            {
                device_info = bt_hfp_hf_app_alloc_device(inst_data);
                if (device_info)
                {
                    device_info->mux_id = msg->mux_id;
                }
            }

            if (device_info)
            {
                device_info->profile_type = msg->profile_type;
                device_info->hfp_bd = msg->bd;
                device_info->pre_profile_state = device_info->profile_state;
                device_info->profile_state = msg->device_state;
                if (bt_hfp_is_support_feature(device_info, HFP_AG_FEAT_ECNR))
                {
                    hfp_hf_send_at_nrec_api(msg->mux_id, HF_CONN);
                    device_info->peer_features &= (~HFP_AG_FEAT_ECNR);
                }

                bt_notify_all_call_status call_info;
                call_info.profile_channel = msg->mux_id;
                call_info.call_status = device_info->cind_status.callStatus;
                call_info.callsetup_status = device_info->cind_status.callSetupStatus;
                call_info.callheld_status = device_info->cind_status.callHeldStatus;
                call_info.roam = device_info->cind_status.roam;
                call_info.service = device_info->cind_status.service;
                call_info.signal = device_info->cind_status.signal;
                call_info.batt_level = device_info->cind_status.batt_level;
                bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_CALL_STATUS_UPDATE,
                                             &call_info, sizeof(bt_notify_all_call_status));
            }

            bt_notify_profile_state_info_t profile_state;
            bt_addr_convert(&msg->bd, profile_state.mac.addr);
            profile_state.profile_type = BT_NOTIFY_HFP_HF;
            profile_state.res = BTS2_SUCC;
            profile_state.profile_channel = msg->mux_id;
            bt_profile_update_connection_state(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_PROFILE_CONNECTED, &profile_state);

            bts2_app_data->bd_list[bts2_app_data->dev_idx] = msg->bd;
            bts2_app_data->last_conn_bd = msg->bd;
            USER_TRACE("conn_cfm bd: %04X:%04X:%04X\n", msg->bd.lap, msg->bd.uap, msg->bd.nap);
            if (msg->profile_type == HF_CONN)
            {
                USER_TRACE("<< HF Conneted success\n");
            }
            else
            {
                USER_TRACE("<< HS Conneted success\n");
            }
#ifdef BTS2_APP_MENU
            bts2_app_data->menu_id = menu_hfp_hf;
            bt_disply_menu(bts2_app_data);
#endif
        }
        else if (msg->device_state == HFP_DEVICE_DISCONNECTED)
        {

            bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(inst_data, msg->mux_id);
            bt_hfp_hf_app_dealloc_device(device_info);

            bt_notify_profile_state_info_t profile_state;
            bt_addr_convert(&msg->bd, profile_state.mac.addr);
            profile_state.profile_type = BT_NOTIFY_HFP_HF;
            profile_state.res = msg->res;
            profile_state.profile_channel = msg->mux_id;
            bt_profile_update_connection_state(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_PROFILE_DISCONNECTED, &profile_state);
        }

        break;
    }

    case BTS2MU_HF_DISC_CFM:
    {
        BTS2S_HF_CONN_CFM *msg;
        msg = (BTS2S_HF_CONN_CFM *)bts2_app_data->recv_msg;
        USER_TRACE("BTS2MU_HF_DISC_CFM state %d res:%x", msg->device_state, msg->res);
        bt_notify_profile_state_info_t profile_state;
        bt_addr_convert(&msg->bd, profile_state.mac.addr);
        profile_state.profile_type = BT_NOTIFY_HFP_HF;
        profile_state.res = msg->res;
        profile_state.profile_channel = msg->mux_id;
        bt_profile_update_connection_state(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_PROFILE_DISCONNECTED,  &profile_state);

        bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(inst_data, msg->mux_id);
        bt_hfp_hf_app_dealloc_device(device_info);
#if defined(AUDIO_USING_MANAGER) && !defined(BT_USING_HF)
        hfp_audio_close_path();
#endif // AUDIO_USING_MANAGER
        break;
    }
    case BTS2MU_HF_DISC_IND:
    {
        BTS2S_HF_CONN_CFM *msg;
        msg = (BTS2S_HF_CONN_CFM *)bts2_app_data->recv_msg;
        USER_TRACE("BTS2MU_HF_DISC_IND state %d res:%x", msg->device_state, msg->res);
        bt_notify_profile_state_info_t profile_state;
        bt_addr_convert(&msg->bd, profile_state.mac.addr);
        profile_state.profile_type = BT_NOTIFY_HFP_HF;
        profile_state.res = msg->res;
        profile_state.profile_channel = msg->mux_id;
        bt_profile_update_connection_state(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_PROFILE_DISCONNECTED, &profile_state);

        bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(inst_data, msg->mux_id);
        bt_hfp_hf_app_dealloc_device(device_info);
#if defined(AUDIO_USING_MANAGER) && !defined(BT_USING_HF)
        hfp_audio_close_path();
#endif // AUDIO_USING_MANAGER
        break;
    }

    case BTS2MU_HF_AT_DATA_IND:
    {
        BTS2S_HF_AT_DATA_IND *msg;
        int i;
        msg = (BTS2S_HF_AT_DATA_IND *)bts2_app_data->recv_msg;

        USER_TRACE("<< Receive Externed AT Command from AG:");
        for (i = 0; i < msg->payload_len; ++i)
        {
            USER_TRACE("%c", msg->payload[i]);
        }

        if (NULL != (msg->payload))
        {
            bfree(msg->payload);
        }
        USER_TRACE("\n");
        break;
    }

    case BTS2MU_HF_SPK_GAIN_IND:
    {
        BTS2S_HF_SPK_GAIN_IND *msg;
        msg = (BTS2S_HF_SPK_GAIN_IND *)bts2_app_data->recv_msg;
        USER_TRACE("<< AG change speaker volume to be %d\n", msg->gain);
        bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + 1);
        data_info->profile_channel = msg->mux_id;
        data_info->payload_len = 1;
        bmemcpy(data_info->payload, &msg->gain, 1);
        bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_VOLUME_CHANGE, data_info, sizeof(bt_notify_ag_at_arg_t) + 1);
        bfree(data_info);

#if defined(AUDIO_USING_MANAGER) && !defined(BT_USING_HF)
        audio_server_set_private_volume(AUDIO_TYPE_BT_VOICE, msg->gain);
#endif
        break;
    }

    case BTS2MU_HF_MIC_GAIN_IND:
    {
        BTS2S_HF_MIC_GAIN_IND *msg;
        msg = (BTS2S_HF_MIC_GAIN_IND *)bts2_app_data->recv_msg;
        USER_TRACE("<< AG change microphone volume to be %d\n", msg->gain);
        break;
    }

    case BTS2MU_HF_VOICE_RECOG_IND:
    {
        BTS2S_HF_VOICE_RECOG_IND *msg;
        msg = (BTS2S_HF_VOICE_RECOG_IND *)bts2_app_data->recv_msg;

        if (msg->val == TRUE)
        {
            USER_TRACE("<< Activate Voice Recognition\n");
        }
        else
        {
            USER_TRACE("<< Deactivate Voice Recognition\n");
        }
        bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + 1);
        data_info->profile_channel = msg->mux_id;
        data_info->payload_len = 1;
        bmemcpy(data_info->payload, &msg->val, 1);
        bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_VOICE_RECOG_STATUS_CHANGE, data_info, sizeof(bt_notify_ag_at_arg_t) + 1);
        bfree(data_info);
        // bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_VOICE_RECOG_STATUS_CHANGE, &msg->val, sizeof(uint8_t));
        break;
    }
    case BTS2MU_HF_AUDIO_CONN_CFM:
    case BTS2MU_HF_AUDIO_DISC_CFM:
    case BTS2MU_HF_AUDIO_DISC_IND:
    {
        BTS2S_HF_AUDIO_INFO *msg;
        msg = (BTS2S_HF_AUDIO_INFO *)bts2_app_data->recv_msg;
        bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(inst_data, msg->mux_id);
        if (msg->audio_on == TRUE)
        {
            USER_TRACE("<< Audio connected\n");
            // sco handle: Packet_Status_Flag inside if any.
            if (device_info)
            {
                device_info->sco_hdl = msg->sco_hdl;
            }
            // gap_reg_sco_callback(msg->sco_hdl, hfp_hf_audio_cb_fn);
            bt_notify_device_sco_info_t sco_info;
            sco_info.sco_type = BT_NOTIFY_HFP_HF;
            sco_info.sco_res = BTS2_SUCC;
            sco_info.para.type = msg->type;
            sco_info.para.mux_id = msg->mux_id;
            sco_info.para.audio_on = msg->audio_on;
            sco_info.para.sco_hdl = msg->sco_hdl;
            sco_info.para.profile_type = msg->profile_type;
            sco_info.para.tx_intvl = msg->tx_intvl;
            sco_info.para.we_sco = msg->we_sco;
            sco_info.para.rx_pkt_len = msg->rx_pkt_len;
            sco_info.para.tx_pkt_len = msg->tx_pkt_len;
            sco_info.para.air_mode = msg->mux_id;
            bt_addr_convert_to_general(&msg->bd, (bd_addr_t *)&sco_info.para.bd);
            bt_interface_bt_event_notify(BT_NOTIFY_COMMON, BT_NOTIFY_COMMON_SCO_CONNECTED,
                                         &sco_info, sizeof(bt_notify_device_sco_info_t));

#if defined(AUDIO_USING_MANAGER) && !defined(BT_USING_HF)
            hfp_set_audio_voice_para(&sco_info.para, msg->audio_on, 1);
#endif // AUDIO_USING_MANAGER

        }
        else
        {
            USER_TRACE("<< Audio disconnect\n");
            if (device_info)
            {
                device_info->sco_hdl = 0xffff;
            }

#if defined(AUDIO_USING_MANAGER) && !defined(BT_USING_HF)
            hfp_audio_close_path();
#endif // AUDIO_USING_MANAGER

            bt_notify_device_sco_info_t sco_info;
            sco_info.sco_type = BT_NOTIFY_HFP_HF;
            sco_info.sco_res = BTS2_SUCC;
            sco_info.profile_channel = msg->mux_id;
            bt_interface_bt_event_notify(BT_NOTIFY_COMMON, BT_NOTIFY_COMMON_SCO_DISCONNECTED,
                                         &sco_info, sizeof(bt_notify_device_sco_info_t));
            gap_unreg_sco_callback(msg->sco_hdl);
        }
        break;
    }
    case BTS2MU_HF_AUDIO_IND:
    {
        break;
    }
    case BTS2MU_HF_AT_CMD_CFM:
    {
        BTS2S_HF_AT_CMD_CFM *msg;
        msg = (BTS2S_HF_AT_CMD_CFM *) bts2_app_data->recv_msg;
        bt_hfp_hf_at_cmd_cfm_hdl(msg);
        break;
    }

    case BTS2MU_HF_RING_IND:
    {
        BTS2S_HF_RING_IND *msg;
        msg = (BTS2S_HF_RING_IND *) bts2_app_data->recv_msg;
        USER_TRACE("<< Ring......\n");
        bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t));
        data_info->profile_channel = msg->mux_id;
        data_info->payload_len = 0;
        bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_RING_IND, data_info, sizeof(bt_notify_ag_at_arg_t));
        bfree(data_info);
        break;
    }
    case BTS2MU_HF_STS_IND:
    {
        BTS2S_HF_ST_IND *msg;
        msg = (BTS2S_HF_ST_IND *) bts2_app_data->recv_msg;

        switch (msg->st_ev)
        {
        case 0:
        {
            USER_TRACE(">> Active mode\n");
            break;
        }
        case 1:
        {
            USER_TRACE(">> Hold mode\n");
            break;
        }
        case 2:
        {
            USER_TRACE(">> Sniff mode\n");
            break;
        }
        case 3:
        {
            USER_TRACE(">> Park mode\n");
            break;
        }
        default:
            break;
        }
        break;
    }
    case BTS2MU_HF_CIEV_IND:
    {
        BTS2S_HF_CIEV_IND *msg;
        U8 callStatus = 0xff;
        msg = (BTS2S_HF_CIEV_IND *) bts2_app_data->recv_msg;
        USER_TRACE(">> \"%s\":%d\n", msg->name, msg->val);
        callStatus = bt_hfp_hf_get_ciev_info(msg);
        break;
    }
    case BTS2MU_HF_CHLD_IND:
    {
        BTS2S_HF_CHLD_IND *msg;
        msg = (BTS2S_HF_CHLD_IND *) bts2_app_data->recv_msg;
        USER_TRACE(">> \"%s\":%d\n", msg->chld_str, msg->supp);
        bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + 1);
        data_info->profile_channel = msg->mux_id;
        data_info->payload_len = 1;
        bmemcpy(data_info->payload, &msg->supp, 1);
        bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_CHLD_IND, data_info, sizeof(bt_notify_ag_at_arg_t) + 1);
        bfree(data_info);
        break;
    }

    case BTS2MU_HF_CLCC_IND:
    {
        BTS2S_HF_CLCC_IND *msg;
        msg = (BTS2S_HF_CLCC_IND *)bts2_app_data->recv_msg;
        /*The numbering (starting with 1) of the call given by the
        sequence of setting up or receiving the calls (active, held
        or waiting) as seen by the served subscriber.*/
        USER_TRACE("<< The index is:%d\n", msg->idx);
        if (msg->dir == 1)
        {
            USER_TRACE("<< It's an incoming call\n");
        }
        else
        {
            USER_TRACE("<< It's an outgoing call\n");
        }
        switch (msg->st)
        {
        case 0:
        {
            USER_TRACE("<< Active\n");
            break;
        }
        case 1:
        {
            USER_TRACE("<< Held\n");
            break;
        }
        case 2:
        {
            USER_TRACE("<< Dialing(outgoing calls only)\n");
            break;
        }
        case 3:
        {
            USER_TRACE("<< Alerting (outgoing calls only)\n");
            break;
        }
        case 4:
        {
            USER_TRACE("<< Incoming (incoming calls only)\n");
            break;
        }
        case 5:
        {
            USER_TRACE("<< Waiting (incoming calls only)\n");
            break;
        }
        default:
        {
            break;
        }
        }
        switch (msg->mode)
        {
        case 0:
        {
            USER_TRACE("<< Voice\n");
            break;
        }
        case 1:
        {
            USER_TRACE("<< Data\n");
            break;
        }
        case 2:
        {
            USER_TRACE("<< Fax\n");
            break;
        }
        default:
        {
            break;
        }
        }
        if (msg->mpty)
        {
            USER_TRACE("<< Multiparty\n");
        }
        else
        {
            USER_TRACE("<< Not Multiparty\n");
        }

        if (0 != msg->data_len)
        {
            bt_notify_clcc_ind_t ind = {0};
            ind.profile_channel = msg->mux_id;
            ind.dir = msg->dir;
            ind.mode = msg->mode;
            ind.mpty = msg->mpty;
            ind.st = msg->st;
            ind.phone_number_type = msg->type;
            ind.number_size = msg->data_len;
            ind.idx = msg->idx;
            bmemcpy(&ind.number, msg->data, ind.number_size);
            bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_REMOTE_CALL_INFO_IND,
                                         &ind, sizeof(bt_notify_clcc_ind_t));
        }
        if (NULL != msg->data)
        {
            bfree(msg->data);
        }

        if (NULL != msg->body)
        {
            bfree(msg->body);
        }
        break;
    }

    case BTS2MU_HF_CNUM_IND:
    {
        BTS2S_HF_CNUM_IND *msg;
        msg = (BTS2S_HF_CNUM_IND *)bts2_app_data->recv_msg;
        USER_TRACE(">> Phone number \"%s\",phone type <%d>\n", msg->phone_number, msg->phone_type);
        bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + msg->phone_len);
        data_info->profile_channel = msg->mux_id;
        data_info->payload_len = msg->phone_len;
        bmemcpy(data_info->payload, msg->phone_number, msg->phone_len);
        bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_LOCAL_PHONE_NUMBER, data_info, sizeof(bt_notify_ag_at_arg_t) + msg->phone_len);
        bfree(data_info);
        break;
    }
    case BTS2MU_HF_CIND_IND:
    {
        BTS2S_HF_CALL_STATUS_IND  *msg;
        msg = (BTS2S_HF_CALL_STATUS_IND *)bts2_app_data->recv_msg;
        USER_TRACE("cind call status %d callHeldStatus %d callSetupStatus %d",
                   msg->cind_status.callStatus, msg->cind_status.callHeldStatus, msg->cind_status.callSetupStatus);

        bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(inst_data, msg->mux_id);
        if (!device_info)
        {
            device_info = bt_hfp_hf_app_alloc_device(inst_data);
            if (device_info)
            {
                device_info->mux_id = msg->mux_id;
            }
        }
        if (device_info)
        {
            device_info->cind_status.callStatus = msg->cind_status.callStatus;
            device_info->cind_status.callHeldStatus = msg->cind_status.callHeldStatus;
            device_info->cind_status.callSetupStatus = msg->cind_status.callSetupStatus;
            device_info->cind_status.roam = msg->cind_status.roam;
            device_info->cind_status.service = msg->cind_status.service;
            device_info->cind_status.signal = msg->cind_status.signal;
            device_info->cind_status.batt_level = msg->cind_status.batt_level;
            break;
        }

        bt_notify_all_call_status call_info;
        call_info.profile_channel = msg->mux_id;
        call_info.call_status = msg->cind_status.callStatus;
        call_info.callsetup_status = msg->cind_status.callSetupStatus;
        call_info.callheld_status = msg->cind_status.callHeldStatus;
        call_info.roam = msg->cind_status.roam;
        call_info.service = msg->cind_status.service;
        call_info.signal = msg->cind_status.signal;
        call_info.batt_level = msg->cind_status.batt_level;
        bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_CALL_STATUS_UPDATE,
                                     &call_info, sizeof(bt_notify_all_call_status));
        break;
    }

    case BTS2MU_HF_BTRH_IND:
    {
        BTS2S_HF_BTRH_IND *msg;
        msg = (BTS2S_HF_BTRH_IND *)bts2_app_data->recv_msg;
        USER_TRACE("URC call 3way btrh ind\n");
        bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + 1);
        data_info->profile_channel = msg->mux_id;
        data_info->payload_len = 1;
        bmemcpy(data_info->payload, &msg->val, 1);
        bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_CALL_RESPONSE_HOLD_IND, data_info, sizeof(bt_notify_ag_at_arg_t) + 1);
        bfree(data_info);
        break;
    }

    case BTS2MU_HF_CLIP_IND:
    {
        BTS2S_HF_CLIP_IND *msg;
        msg = (BTS2S_HF_CLIP_IND *)bts2_app_data->recv_msg;
        USER_TRACE("CLIP call phone number: %s,phone type %d", msg->phone_number, msg->phone_type);
        bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + msg->phone_len);
        data_info->profile_channel = msg->mux_id;
        data_info->payload_len = msg->phone_len;
        bmemcpy(data_info->payload, msg->phone_number, msg->phone_len);
        bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_CALL_INCOMING_NUMBER_IND, data_info, sizeof(bt_notify_ag_at_arg_t) + msg->phone_len);
        bfree(data_info);
        break;
    }

    case BTS2MU_HF_CCWA_IND:
    {
        BTS2S_HF_CCWA_IND *msg;
        msg = (BTS2S_HF_CCWA_IND *)bts2_app_data->recv_msg;
        USER_TRACE("CCWA Phone number: %s phone type: %d", msg->phone_number, msg->phone_type);
        bt_notify_ag_at_arg_t *data_info = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t) + msg->phone_len);
        data_info->profile_channel = msg->mux_id;
        data_info->payload_len = msg->phone_len;
        bmemcpy(data_info->payload, msg->phone_number, msg->phone_len);
        bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_CALL_INCOMING_NUMBER_IND, data_info, sizeof(bt_notify_ag_at_arg_t) + msg->phone_len);
        bfree(data_info);
        break;
    }

    case BTS2MU_HF_BINP_IND:
    {
        BTS2S_HF_BINP_IND *msg;

        msg = (BTS2S_HF_BINP_IND *)bts2_app_data->recv_msg;
        if (msg->res == BTS2_SUCC)
        {
            int i = 0;
            USER_TRACE("-- Phone number: ");
            for (i = 0; i < msg->phone_len; i++)
            {
                USER_TRACE("%c", msg->phone_number[i]);
            }
            USER_TRACE("-- phone type <%d>\n", msg->phone_type);
        }
        else
        {
            USER_TRACE("-- ERROR\n");
        }
        break;
    }

    case BTS2MU_HF_CMEE_IND:
    {
        BTS2S_HF_CMEE_IND *msg;
        msg = (BTS2S_HF_CMEE_IND *)bts2_app_data->recv_msg;
        USER_TRACE(">> Error code is: %d\n", msg->cmee_err_code);
        break;
    }

    case BTS2MU_HF_COPP_SRV_QUERY_IND:
    {
        BTS2MD_HF_COPP_SRV_QUERY_IND *msg;
        int i = 0;

        msg = (BTS2MD_HF_COPP_SRV_QUERY_IND *)bts2_app_data->recv_msg;
        USER_TRACE("<< Network operator's name is: ");
        for (i = 0; i < msg->data_len; i++)
        {
            USER_TRACE("%c", msg->data[i]);
        }
        USER_TRACE("\n");
        break;
    }
    case BTS2MU_HF_BSIR_IND:
    {
        BTS2S_HF_COMMON_CFM *msg;
        msg = (BTS2S_HF_COMMON_CFM *)bts2_app_data->recv_msg;
        USER_TRACE("BTS2MU_HF_BSIR_IND %d\n", msg->res);
        bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(inst_data, msg->mux_id);
        if (device_info)
        {
            if (msg->res)
            {
                device_info->peer_features |= HFP_AG_FEAT_INBAND;
            }
            else
            {
                device_info->peer_features &= (~HFP_AG_FEAT_INBAND);
            }
            bt_notify_ag_at_arg_t *bsir = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t));
            bsir->profile_channel = msg->mux_id;
            bsir->payload_len = 1;
            bsir->payload[0] = bt_hfp_is_support_feature(device_info, HFP_AG_FEAT_INBAND);
            bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_INBAND_RING_STATUS_CHANGE, bsir, sizeof(bt_notify_ag_at_arg_t));
            bfree(bsir);
        }
        break;
    }

    case BTS2MU_HF_BRSF_IND:
    {
        BTS2S_HF_BRSF_IND *msg;
        msg = (BTS2S_HF_BRSF_IND *)bts2_app_data->recv_msg;
        bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(inst_data, msg->mux_id);
        if (!device_info)
        {
            device_info = bt_hfp_hf_app_alloc_device(inst_data);
            if (device_info)
            {
                device_info->mux_id = msg->mux_id;
            }
        }

        if (device_info)
        {
            device_info->peer_features = msg->supp_feature;
            bt_notify_ag_at_arg_t *supp_voice_reg = (bt_notify_ag_at_arg_t *)bmalloc(sizeof(bt_notify_ag_at_arg_t));
            supp_voice_reg->profile_channel = msg->mux_id;
            supp_voice_reg->payload_len = 1;
            supp_voice_reg->payload[0] = bt_hfp_is_support_feature(device_info, HFP_AG_FEAT_VREC);
            bt_interface_bt_event_notify(BT_NOTIFY_HFP_HF, BT_NOTIFY_HF_VOICE_RECOG_STATUS_CHANGE, supp_voice_reg, sizeof(bt_notify_ag_at_arg_t));
            bfree(supp_voice_reg);
        }
        break;
    }
    default:
    {
        USER_TRACE("<< Unexpected message %x\n", *msg_type);
        break;
    }
    }
}

U8 bt_hfp_hf_get_ring_type_ext(uint8_t mux_id)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    return bt_hfp_is_support_feature(device_info, HFP_AG_FEAT_INBAND);
}

bt_err_t bt_hfp_hf_audio_transfer_request(U8 mux_id, U8 type)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            // type 0:connect audio type 1 :disconnect audio
            if (type == 0)
            {
                hfp_hf_connect_audio(&device_info->hfp_bd, 0);//connect audio
                ret = BT_EOK;
            }
            else if (type == 1)
            {
                hfp_hf_disconnect_audio(&device_info->hfp_bd, 0);//disconect audio
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
        {
            USER_TRACE(">> Transfer audio path fail\n");
            break;
        }
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_voice_recog_request(U8 mux_id, U8 active)
{
    bt_err_t ret = BT_ERROR_STATE;
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (active == 0 || active == 1)
            {
                hfp_hf_send_at_bvra_api(mux_id, HF_CONN, active);
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_dial_by_mem_request(U8 mux_id, U16 memory)
{
    bt_err_t ret = BT_ERROR_STATE;
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            char data[6];
            int at_len = 0;
            at_len = snprintf(data, sizeof(data), ">%u;", memory);
            hfp_hf_send_at_atd_api(mux_id, HF_CONN, (U8 *)data, (U8) at_len);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_last_num_dial_request(U8 mux_id, U16 memory)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_bldn_api(mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }

    return ret;
}

bt_err_t bt_hfp_hf_make_call_by_number_request(U8 mux_id, U8 *payload, U8 payload_len)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            char *data;
            int p_payload_len = payload_len + 1;
            data = (char *)bmalloc(p_payload_len);
            if (data)
            {
                bmemcpy(data, payload, payload_len);
                data[payload_len] = ';';
                hfp_hf_send_at_atd_api(mux_id, HF_CONN, (U8 *)data, (U8) p_payload_len);
                USER_TRACE("data %s len %d input_len %d", data, p_payload_len, payload_len);
                ret = BT_EOK;
                bfree(data);
            }
            else
                ret = BT_ERROR_OUT_OF_MEMORY;
            break;
        }
        default:
        {
            USER_TRACE(">> dial out fail\n");
            break;
        }
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_answer_call_request(U8 mux_id)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_ata_api(mux_id, HF_CONN);
            USER_TRACE(">> Answer the incoming call\n");
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_hangup_call_request(U8 mux_id)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_chup_api(mux_id, HF_CONN);
            USER_TRACE(">> hfp_hf reject terminal the call\n");
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}
bt_err_t bt_hfp_hf_update_spk_vol_ext(U8 mux_id, U8 vol)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);

    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (0 <= vol && vol <= 15)
            {
                hfp_hf_send_at_vgs_api(mux_id, HF_CONN, (U8)vol); //just send 0---15
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_update_mic_vol_ext(U8 mux_id, U8 vol)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (0 <= vol && vol <= 15)
            {
                hfp_hf_send_at_vgm_api(mux_id, HF_CONN, vol); //just send 0---15
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_btrh_query_request(U8 mux_id)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);

    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_btrh_api(mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_btrh_cmd_request(U8 mux_id, U8 mode)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    USER_TRACE(">> enter \n");
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_btrh_mode_api(mux_id, HF_CONN, mode);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_binp_request(U8 mux_id)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            //Attach a Phone Number to a Voice Tag.
            hfp_hf_send_at_binp_api(mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_clip_request(U8 mux_id, U8 enable)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            //Enable calling Line Identification (CLI) Notification.
            if (enable == 0 || enable == 1)
            {
                hfp_hf_send_at_clip_api(mux_id, HF_CONN, enable);
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}
bt_err_t bt_hfp_hf_at_cmee_request(U8 mux_id, BOOL val)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (val == 0 || val == 1)
            {
                hfp_hf_send_at_cmee_api(mux_id, HF_CONN, val);
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_cnum_request(U8 mux_id)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_cnum_api(mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_ccwa_request(U8 mux_id, BOOL val)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (val == 0 || val == 1)
            {
                hfp_hf_send_at_ccwa_api(mux_id, HF_CONN, val); //active
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_chld_request(U8 mux_id, U8 *payload, U8 payload_len)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_chld_control_api(mux_id, HF_CONN, payload, payload_len);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_clcc_request(U8 mux_id)
{
    bt_err_t ret = BT_ERROR_STATE;
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);

    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_clcc_api(mux_id, HF_CONN);
            // ok
            //during a call process, solution can send clcc the get info. so ptr->st maybe hfp_conned or hfp_calling
            USER_TRACE(">> List current call status\n");
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_cops_cmd_request(U8 mux_id)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;

    if (device_info && (device_info->profile_state == HFP_DEVICE_CONNECTED))
    {
        //hfp_hf_copp_srv_req(COPSMODE, COPSFMTE);
        char *payload =  "3,0";
        U8 payload_len = strlen(payload);
        hfp_hf_send_at_cops_cmd_api(mux_id, HF_CONN, (U8 *)payload, payload_len);
        ret = BT_EOK;
        USER_TRACE(">> set the cops information\n");
    }
    else
    {
        USER_TRACE(">> Not in connected state\n");
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_dtmf_request(U8 mux_id, char key)
{
    //U8 *data;
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_vts_api(mux_id, HF_CONN, key);
            ret = BT_EOK;
            break;
        }
        default:
            USER_TRACE("-- Hf state error, current state is %d\n", device_info->profile_state);
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_at_nrec_request(U8 mux_id)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            //The HF may disable the echo canceling and noise reduction functions resident in the AG via the AT+NREC command.
            hfp_hf_send_at_nrec_api(mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t bt_hfp_hf_update_batt_request(U8 mux_id, U8 batt_val)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            if (0 <= batt_val && batt_val <= 9)
            {
                char data[8];
                int at_len = 0;
                at_len = snprintf(data, sizeof(data), "1,1,%d", batt_val);
                hfp_hf_send_at_batt_update_api(mux_id, HF_CONN, (U8 *)data, (U8) at_len); //just 0~9
                ret = BT_EOK;
            }
            else
            {
                ret = BT_ERROR_INPARAM;
            }
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

bt_err_t hfp_hf_get_at_cind_status_ext(U8 mux_id)
{
    bts2_hfp_hf_inst_data *hf_data = bt_hfp_hf_get_context();
    bts2_hfp_hf_device_info *device_info = bt_hfp_hf_app_get_device_by_mux_id(hf_data, mux_id);
    bt_err_t ret = BT_ERROR_STATE;
    if (device_info)
    {
        switch (device_info->profile_state)
        {
        case HFP_DEVICE_CONNECTED:
        {
            hfp_hf_send_at_cind_status_api(mux_id, HF_CONN);
            ret = BT_EOK;
            break;
        }
        default:
            break;
        }
    }
    return ret;
}
#endif




