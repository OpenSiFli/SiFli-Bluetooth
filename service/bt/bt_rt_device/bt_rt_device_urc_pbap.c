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

#define DBG_TAG               "bt_rt_device.urc_pbap"
//#define DBG_LVL               DBG_INFO
#include <log.h>


void urc_func_pbap_vcard_list_notify(pbap_vcard_listing_item_t *msg)
{
    bt_notify_t args;
    args.event = BT_EVENT_VCARD_LIST_ITEM_NOTIFY;
    args.args = msg;
    rt_bt_event_notify(&args);
    LOG_I("URC bt vcard_list notify %s len:%d", msg->vcard_name, msg->name_len);
}

void urc_func_pbap_vcard_list_cmp(U8 res)
{
    bt_notify_t args;
    args.event = BT_EVENT_VCARD_LIST_CMP;
    args.args = &res;
    rt_bt_event_notify(&args);
    LOG_I("URC bt vcard_list notify cmp %d", res);
}

int bt_sifli_notify_pbap_event_hdl(uint16_t event_id, uint8_t *data, uint16_t data_len)
{
    switch (event_id)
    {
    case BT_NOTIFY_PBAP_PROFILE_CONNECTED:
    {
        bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
        urc_func_profile_conn_sifli(profile_info->mac.addr, BT_PROFILE_PBAP);
        break;
    }
    case BT_NOTIFY_PBAP_PROFILE_DISCONNECTED:
    {
        bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
        urc_func_profile_disc_sifli(profile_info->mac.addr, BT_PROFILE_PBAP, profile_info->res);
        break;
    }
    case BT_NOTIFY_PBAP_VCARD_LIST_ITEM_IND:
    {
        pbap_vcard_listing_item_t *list_item = (pbap_vcard_listing_item_t *)data;
        urc_func_pbap_vcard_list_notify(list_item);
        break;
    }
    case BT_NOTIFY_PBAP_VCARD_LIST_CMPL:
    {
        urc_func_pbap_vcard_list_cmp(data[0]);
        break;
    }
    default:
        return -1;
    }
    return 0;
}


