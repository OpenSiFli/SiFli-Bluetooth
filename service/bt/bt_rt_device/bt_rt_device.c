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
#include "bt_device.h"
#include "bt_rt_device.h"
#include "bts2_app_inc.h"
#include "bf0_ble_common.h"
#include "bf0_sibles.h"
#include "bt_connection_manager.h"


static const struct rt_bt_ops bt_sifli_ops =
{
    .control = bt_sifli_control
};

BOOL bt_receive_connect_req_handler(BTS2S_BD_ADDR *p_bd, U24 dev_cls)
{
#ifdef BT_CONNECT_SUPPORT_MULTI_LINK
    rt_bt_device_t *bt_device = (rt_bt_device_t *) rt_device_find(BT_DEVICE_NAME);
    bt_cm_bonded_dev_t *bond_info = bt_cm_get_bonded_dev();
    if (dev_cls & BT_DEVCLS_PHONE)
    {
        for (uint8_t i = 0; i < BT_MAX_ACL_NUM; i++)
        {
            if (bond_info->info[i].role == BT_CM_SLAVE && BT_STATE_ACL_CONNECTED == rt_bt_get_acl_state_by_conn_idx(bt_device, i))
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    else if (dev_cls & (BT_SRVCLS_AUDIO | BT_SRVCLS_RENDER | BT_DEVCLS_AUDIO))
    {
        for (uint8_t i = 0; i < BT_MAX_ACL_NUM; i++)
        {
            if (bond_info->info[i].role == BT_CM_MASTER && BT_STATE_ACL_CONNECTED == rt_bt_get_acl_state_by_conn_idx(bt_device, i))
            {
                return FALSE;
            }
        }
    }
#endif
    bd_addr_t dest_addr = {0};
    bt_addr_convert_to_general(p_bd, &dest_addr);
    if (bt_cm_find_bonded_dev_by_addr(dest_addr.addr) == NULL)
    {
        return FALSE;
    }
    return TRUE;
}


int bt_sifli_init(void)
{
    bt_register_receive_connect_req_handler(bt_receive_connect_req_handler);
    rt_hw_bt_init(&bt_sifli_ops, BT_DEVICE_FLAG_OPEN);
    return BT_EOK;
}

INIT_COMPONENT_EXPORT(bt_sifli_init);



