/*
 * SPDX-FileCopyrightText: 2022-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>
#include <string.h>
#include "drv_bt.h"
//#include "utf8_unicode.h"
#include "bt_rt_device.h"
#include "bts2_global.h"
#include "bts2_app_inc.h"

#define DBG_TAG               "bt_rt_device.urc_ag"
//#define DBG_LVL               DBG_INFO
#include <log.h>


void urc_func_ag_answer_call_req(void)
{
    bt_notify_t args;
    args.event = BT_EVENT_AG_ANSWER_CALL_REQ;
    args.args = RT_NULL;
    rt_bt_event_notify(&args);
    LOG_I("URC AG BT_EVENT_AG_ANSWER_CALL_REQ");
}

void urc_func_ag_hangup_call_req(void)
{
    bt_notify_t args;
    args.event = BT_EVENT_AG_HUNGUP_CALL_REQ;
    args.args = RT_NULL;
    rt_bt_event_notify(&args);
    LOG_I("URC AG BT_EVENT_AG_HUNGUP_CALL_REQ");
}

void urc_func_ag_make_call_req(char *phone_number)
{
    bt_notify_t args;
    args.event = BT_EVENT_MAKE_CALL_REQ;
    args.args = phone_number;
    rt_bt_event_notify(&args);
    LOG_I("URC AG phone num:%s", phone_number);
}

void urc_func_ag_dmtf_key_req(char val)
{
    bt_notify_t args;
    args.event = BT_EVENT_DTMF_KEY_REQ;
    args.args = &val;
    rt_bt_event_notify(&args);
    LOG_I("URC AG dmtf key:%c", val);
}

void urc_func_ag_get_local_phone_info_req(void)
{
    bt_notify_t args;
    args.event = BT_EVENT_GET_LOCAL_PHONE_INFO_REQ;
    args.args = RT_NULL;
    rt_bt_event_notify(&args);
    LOG_I("URC AG BT_EVENT_GET_LOCAL_PHONE_INFO_REQ");
}

void urc_func_ag_get_indicator_status_req(void)
{
    bt_notify_t args;
    args.event = BT_EVENT_GET_INDICATOR_STATUS_REQ;
    args.args = RT_NULL;
    rt_bt_event_notify(&args);
    LOG_I("URC AG BT_EVENT_GET_INDICATOR_STATUS_REQ");
}

void urc_func_ag_get_all_call_status_req(void)
{
    bt_notify_t args;
    args.event = BT_EVENT_GET_ALL_REMOTE_CALL_INFO_REQ;
    args.args = RT_NULL;
    rt_bt_event_notify(&args);
    LOG_I("URC AG BT_EVENT_GET_ALL_REMOTE_CALL_INFO_REQ");
}

RT_WEAK void urc_func_bt_voice_volume_sifli(uint8_t volume)
{
    bt_notify_t args;
    bt_volume_set_t vol = {0};
    vol.mode = BT_VOLUME_CALL;
    vol.volume.call_volume = volume;
    args.event = BT_EVENT_VOL_CHANGED;
    args.args = &vol;
    rt_bt_event_notify(&args);
    LOG_I("URC BT hfp-volume ind:%d", volume);
}

int bt_sifli_notify_hfp_ag_event_hdl(uint16_t event_id, uint8_t *data, uint16_t data_len)
{
    switch (event_id)
    {
    case BT_NOTIFY_AG_PROFILE_CONNECTED:
    {
        bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
        urc_func_profile_conn_sifli(profile_info->mac.addr, BT_PROFILE_HFP);
        break;
    }
    case BT_NOTIFY_AG_PROFILE_DISCONNECTED:
    {
        bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
        urc_func_profile_disc_sifli(profile_info->mac.addr, BT_PROFILE_HFP, profile_info->res);
        break;
    }
    case BT_NOTIFY_AG_MAKE_CALL_REQ:
    {
        urc_func_ag_make_call_req((char *)data);
        break;
    }
    case BT_NOTIFY_AG_ANSWER_CALL_REQ:
    {
        urc_func_ag_answer_call_req();
        break;
    }
    case BT_NOTIFY_AG_HANGUP_CALL_REQ:
    {
        urc_func_ag_hangup_call_req();
        break;
    }
    case BT_NOTIFY_AG_RECV_DTMF_KEY:
    {
        urc_func_ag_dmtf_key_req((char)data[0]);
        break;
    }
    case BT_NOTIFY_AG_VOLUME_CHANGE:
    {
        urc_func_bt_voice_volume_sifli(data[0]);
        break;
    }
    case BT_NOTIFY_AG_GET_INDICATOR_STATUS_REQ:
    {
        urc_func_ag_get_indicator_status_req();
        break;
    }
    case BT_NOTIFY_AG_GET_ALL_REMT_CALLS_INFO_REQ:
    {
        urc_func_ag_get_all_call_status_req();
        break;
    }
    case BT_NOTIFY_AG_GET_LOCAL_PHONE_INFO_REQ:
    {
        urc_func_ag_get_local_phone_info_req();
        break;
    }
    case BT_NOTIFY_AG_EXTERN_AT_CMD_KEY_REQ:
    {
        break;
    }
    default:
        return -1;
    }
    return 0;
}


