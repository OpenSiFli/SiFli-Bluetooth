/*
 * SPDX-FileCopyrightText: 2025-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
/* User code start from here --------------------------------------------------------*/
#include "bts2_app_inc.h"

#ifdef CFG_HFP_CALL_STATUS
#include "bt_connection_manager.h"
#include "ulog.h"

#include "hfp_call.h"
#include "hfp_call_service.h"

typedef struct
{
    uint8_t         is_use;
    bt_notify_device_mac_t mac;
    hfp_link_data   link_data;
} bt_hfp_data_t;

hf_call_notify_func_t       g_call_status_cb = NULL;

static rt_mq_t g_bt_hfp_service_queue;
static struct rt_thread g_bt_hfp_service_thread;
uint8_t bt_hfp_service_thread_stack[3072];
bt_hfp_data_t g_hfp_connext[CFG_MAX_HFP_CONN_NUM];

/*******************************************device info func start**********************************************/
static void bt_hfp_init_hf_data_info(bt_hfp_data_t *hf_data)
{
    if (hf_data)
    {
        for (int i = 0; i < CFG_MAX_HFP_CONN_NUM; i++)
        {
            bmemset(&hf_data[i], 0x00, sizeof(bt_hfp_data_t));
            hf_data[i].is_use = 1;
            hf_data[i].link_data.mux_id = 0xff;
        }
    }
}

static bt_hfp_data_t *bt_hfp_get_hf_data_by_mux_id(bt_hfp_data_t *hf_data, U8 mux_id)
{
    if (hf_data)
    {
        for (int i = 0; i < CFG_MAX_HFP_CONN_NUM; i++)
        {
            if (hf_data[i].link_data.mux_id == mux_id)
            {
                return &hf_data[i];
            }
        }
    }
    return NULL;
}

static bt_hfp_data_t *bt_hfp_alloc_hf_data(bt_hfp_data_t *hf_data)
{
    if (hf_data)
    {
        for (int i = 0; i < CFG_MAX_HFP_CONN_NUM; i++)
        {
            if (hf_data[i].is_use)
            {
                hf_data[i].is_use = 0;
                hf_data[i].link_data.callback = g_call_status_cb;
                return &hf_data[i];
            }
        }
    }
    return NULL;
}

static void bt_hfp_dealloc_hf_data(bt_hfp_data_t *device)
{
    if (device)
    {
        bmemset(device, 0x00, sizeof(bt_hfp_data_t));
        device->is_use = 1;
        device->link_data.mux_id = 0xff;
    }
}
/*******************************************device info func end**********************************************/
static void bt_sifli_notify_hdl_at_cmd(uint16_t profile_channel, uint8_t cmd_id, uint8_t res)
{
    bt_hfp_data_t *hf_data = bt_hfp_get_hf_data_by_mux_id(g_hfp_connext, profile_channel);
    switch (cmd_id)
    {
    case HFP_HF_AT_CIND_STATUS:
    {
        LOG_I("get remote all call status complete %d", res);
        break;
    }
    case HFP_HF_AT_CHLD_CMD:
    {
        LOG_I("control remote three_waiting call complete %d", res);
        break;
    }
    case HFP_HF_AT_CLIP:
    {
        LOG_I("open remote incoming call notify complete %d", res);
        break;
    }
    case HFP_HF_AT_CCWA:
    {
        LOG_I("open remote the second incoming call notify complete %d", res);
        break;
    }
    case HFP_HF_AT_BVRA:
    {
        LOG_I("start / stop phone voice recognition complete %d", res);
        if (hf_data)
        {
            hfp_call_dial_complete_hdl(&hf_data->link_data, res);
        }
        break;
    }
    case HFP_HF_AT_CLCC:
    {
        LOG_I("get remote all call information complete %d", res);
        break;
    }
    case HFP_HF_AT_ATA:
    {
        LOG_I("answer a call complete %d", res);
        break;
    }
    case HFP_HF_AT_CHUP:
    {
        LOG_I("hangup a call complete %d", res);
        break;
    }
    case HFP_HF_AT_ATD:
    {
        LOG_I("make a call complete %d", res);
        if (hf_data)
        {
            hfp_call_dial_complete_hdl(&hf_data->link_data, res);
        }
        break;
    }
    case HFP_HF_AT_BLDN:
    {
        LOG_I("make a callback complete %d", res);
        if (hf_data)
        {
            hfp_call_dial_complete_hdl(&hf_data->link_data, res);
        }
        break;
    }
    case HFP_HF_AT_VTS:
    {
        LOG_I("send a DTMF key complete %d", res);
        break;
    }
    case HFP_HF_AT_VGS:
    {
        LOG_I("change volume value complete %d", res);
        break;
    }
    case HFP_HF_AT_BCC:
    {
        break;
    }
    case HFP_HF_AT_CNUM:
    {
        LOG_I("get remote local phone number complete %d", res);
        break;
    }
    case HFP_HF_AT_BATT_UPDATE:
    {
        LOG_I("update local battery level complete %d", res);
        break;
    }
    default:
        break;
    }
}

static int bt_hfp_service_notify_event_handle(uint16_t type, uint16_t event_id, uint8_t *data, uint16_t data_len)
{
    if (type == BT_NOTIFY_COMMON)
    {
        switch (event_id)
        {
        case BT_NOTIFY_COMMON_BT_STACK_READY:
        {
            break;
        }
        case BT_NOTIFY_COMMON_DISCOVER_IND:
        {
            break;
        }
        case BT_NOTIFY_COMMON_INQUIRY_CMP:
        {
            LOG_I("Inquiry completed");
            break;
        }
        case BT_NOTIFY_COMMON_SCO_CONNECTED:
        {
            //handle hf sco conencted msg(bt_notify_device_sco_info_t *)
            LOG_I("HFP HF audio_connected");
            break;
        }
        case BT_NOTIFY_COMMON_SCO_DISCONNECTED:
        {
            //handle sco disconneted msg(bt_notify_device_sco_info_t *)
            LOG_I("HFP HF audio_disconnected");
            break;
        }
        default:
            break;
        }
    }
    else if (type == BT_NOTIFY_HFP_HF)
    {
        switch (event_id)
        {
        case BT_NOTIFY_HF_PROFILE_CONNECTED:
        {
            bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
            LOG_I("HFP HF connected");
            bt_hfp_data_t *hf_data = bt_hfp_alloc_hf_data(g_hfp_connext);
            if (hf_data)
            {
                hf_data->mac = profile_info->mac;
                hf_data->link_data.mux_id = profile_info->profile_channel;
            }
            break;
        }
        case BT_NOTIFY_HF_PROFILE_DISCONNECTED:
        {
            bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
            bt_hfp_data_t *hf_data = bt_hfp_get_hf_data_by_mux_id(g_hfp_connext, profile_info->profile_channel);
            if (hf_data)
            {
                bt_hfp_dealloc_hf_data(hf_data);
            }
            LOG_I("HFP HF disconnected");
            break;
        }
        case BT_NOTIFY_HF_VOICE_RECOG_CAP_UPDATE:
        {
            //msg->data[0]
            if (data[0])
            {
                LOG_I("remote device support voice recognition");
            }
            else
            {
                LOG_I("remote device dont support voice recognition");
            }
            break;
        }
        case BT_NOTIFY_HF_VOICE_RECOG_STATUS_CHANGE:
        {
            if (data[0])
            {
                LOG_I("remote device voice recognition status is on");
            }
            else
            {
                LOG_I("remote device voice recognition status is off");
            }
            break;
        }
        case BT_NOTIFY_HF_LOCAL_PHONE_NUMBER:
        {
            // msg->data  msg->data_len
            if (data_len)
            {
                LOG_I("the remote phone local number:%s", data);
            }
            break;
        }
        case BT_NOTIFY_HF_REMOTE_CALL_INFO_IND:
        {
            bt_notify_clcc_ind_t *clcc_info = (bt_notify_clcc_ind_t *)data;
            if (data_len)
            {
                LOG_I("the remote phone call info phone_number_type:%d", clcc_info->phone_number_type);
                LOG_I("the remote phone call info call_idx:%d", clcc_info->idx);
                LOG_I("the remote phone call info call_direction:%d", clcc_info->dir);
                LOG_I("the remote phone call info call_status:%d", clcc_info->st);
                LOG_I("the remote phone call info call_mode:%d", clcc_info->mode);
                LOG_I("the remote phone call info call_mpty:%d", clcc_info->mpty);
                LOG_I("the remote phone call info call_number_size:%d", clcc_info->number_size);
                LOG_I("the remote phone call info call_number:%s", clcc_info->number);
                bt_hfp_data_t *hf_data = bt_hfp_get_hf_data_by_mux_id(g_hfp_connext, clcc_info->profile_channel);
                if (hf_data)
                {
                    hfp_call_current_indication_hdl(&hf_data->link_data, clcc_info->mpty);
                }
            }
            break;
        }
        case BT_NOTIFY_HF_VOLUME_CHANGE:
        {
            LOG_I("the remote phone want to change volume be: %d", data[0]);
            break;
        }
        case BT_NOTIFY_HF_CALL_STATUS_UPDATE:
        {
            bt_notify_all_call_status *call_status = (bt_notify_all_call_status *)data;
            LOG_I("the remote phone call_status: %d", call_status->call_status);
            LOG_I("the remote phone callsetup_status: %d", call_status->callsetup_status);
            LOG_I("the remote phone callheld_status: %d", call_status->callheld_status);
            bt_hfp_data_t *hf_data = bt_hfp_get_hf_data_by_mux_id(g_hfp_connext, call_status->profile_channel);
            if (hf_data)
            {
                hfp_call_indication_hdl(&hf_data->link_data, call_status->call_status);
                if (call_status->callsetup_status)
                {
                    hfp_callsetup_indication_hdl(&hf_data->link_data, call_status->callsetup_status);
                }
                if (call_status->callheld_status)
                {
                    hfp_callheld_indication_hdl(&hf_data->link_data, call_status->callheld_status);
                }
            }
            break;
        }
        case BT_NOTIFY_HF_INDICATOR_UPDATE:
        {
            bt_notify_cind_ind_t   *cind_status = (bt_notify_cind_ind_t *)data;

            bt_hfp_data_t *hf_data = bt_hfp_get_hf_data_by_mux_id(g_hfp_connext, cind_status->profile_channel);
            if (hf_data)
            {
                if (cind_status->type == HFP_AG_CIND_CALL_TYPE)
                {
                    hfp_call_indication_hdl(&hf_data->link_data, cind_status->val);
                }
                else if (cind_status->type == HFP_AG_CIND_CALLSETUP_TYPE)
                {
                    hfp_callsetup_indication_hdl(&hf_data->link_data, cind_status->val);
                }
                else if (cind_status->type == HFP_AG_CIND_CALLHELD_TYPE)
                {
                    hfp_callheld_indication_hdl(&hf_data->link_data, cind_status->val);
                }
            }
            LOG_I("the remote phone call status type:%d, status%d: %d", cind_status->type, cind_status->val);
            break;
        }
        case BT_NOTIFY_HF_AT_CMD_CFM:
        {
            bt_notify_at_cmd_cfm_t *at_cmd_cfm = (bt_notify_at_cmd_cfm_t *) data;
            bt_sifli_notify_hdl_at_cmd(at_cmd_cfm->profile_channel, at_cmd_cfm->at_cmd_id, at_cmd_cfm->res);
            break;
        }
        }
    }
    return 0;
}

/**
 * @brief Common initialization.
 */
rt_err_t hfp_service_init(void)
{

    bt_hfp_init_hf_data_info(g_hfp_connext);
    bt_interface_register_bt_event_notify_callback(bt_hfp_service_notify_event_handle);
    return RT_EOK;
}

int hfp_call_status_register_callback(hf_call_notify_func_t cb)
{
    if (cb == NULL)
    {
        LOG_I("[HF] Error: callback is NULL\n");
        return -1;
    }

    if (!g_call_status_cb)
    {
        g_call_status_cb = cb;
        hfp_service_init();
        LOG_I("hfp_service register success");
    }

    return 0;
}

/**
 * @brief 注销通话状态回调
 */
int hfp_call_status_unregister_callback(void)
{
    g_call_status_cb = NULL;
    bt_interface_unregister_bt_event_notify_callback(bt_hfp_service_notify_event_handle);
    LOG_I("hfp_service unregister success");
    return 0;
}
#endif