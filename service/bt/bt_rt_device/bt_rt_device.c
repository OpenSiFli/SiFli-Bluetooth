/**
  ******************************************************************************
  * @file   bt_rt_device.c
  * @author Sifli software development team
 *
  ******************************************************************************
*/
/**
 * @attention
 * Copyright (c) 2022 - 2022,  Sifli Technology
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Sifli integrated circuit
 *    in a product or a software update for such product, must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Sifli nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Sifli integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY SIFLI TECHNOLOGY "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SIFLI TECHNOLOGY OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
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

/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/

