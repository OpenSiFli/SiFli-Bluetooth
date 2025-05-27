/**
  ******************************************************************************
  * @file   bt_lwip_pan_dev.c
  * @author Sifli software development team
  ******************************************************************************
*/
/*
 * @attention
 * Copyright (c) 2023 - 2025,  Sifli Technology
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
#ifndef __BT_DEV_PAN_H__
#define __BT_DEV_PAN_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <rthw.h>
#include <rtthread.h>
#include "lwip/pbuf.h"
#include "rtdef.h"
#include "bts2_app_inc.h"
#include "bt_prot.h"


rt_err_t bt_lwip_pan_dev_send_data(struct rt_bt_lwip_pan_dev *bt_dev, void *buff, int len);

struct rt_bt_lwip_pan_dev  bt_lwip_pan_dev[MAX_PAN_INSTANCE_NUM];

struct rt_bt_dev_ops dev_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    bt_lwip_pan_dev_send_data
};

struct rt_bt_lwip_pan_dev * bt_lwip_get_default_pan_dev()
{
    return &bt_lwip_pan_dev[0];
}

void bt_lwip_pan_attach_tcpip()
{
    bt_lwip_pan_dev[0].ops =  &dev_ops;
    rt_bt_prot_attach_pan_dev(bt_lwip_get_default_pan_dev());
}

void bt_lwip_pan_detach_tcpip()
{
    rt_bt_prot_detach_pan_dev(bt_lwip_get_default_pan_dev());
}

void rt_lwip_instance_register_event_handler(struct rt_bt_lwip_pan_dev *bt_dev, rt_bt_dev_event_t event, rt_bt_pan_dev_event_handler handler)
{
    int i = 0;
    for (i = 0; i < MAX_PAN_INSTANCE_NUM; i++)
    {
        bt_dev->handler_table[event][i] = handler;
    }
}

void bt_lwip_pan_dev_recv_data(void *buff, int len)
{
    struct rt_bt_lwip_pan_dev * bt_dev = bt_lwip_get_default_pan_dev();

    if(bt_dev)
    {
        rt_bt_prot_recv_data(bt_dev, buff, len);
    }
}

extern void rt_bt_lwip_event_handle(struct rt_bt_lwip_pan_dev *bt_dev, int event);
rt_err_t bt_pan_connect_event_handle(uint16_t event_id)
 {
    
    switch (event_id)
    {
    case BT_NOTIFY_PAN_PROFILE_CONNECTED:
    {
        struct rt_bt_lwip_pan_dev * bt_dev = bt_lwip_get_default_pan_dev();

        if (bt_dev && bt_dev->prot)
        {
            rt_bt_lwip_event_handle(bt_dev, RT_BT_PROT_EVT_CONNECT);
        }
        else
        {
            rt_bt_prot_attach_pan_dev(bt_dev);
            RT_ASSERT(bt_dev->prot == NULL);
            rt_bt_lwip_event_handle(bt_dev, RT_BT_PROT_EVT_CONNECT);
        }

        break;
    }
    case BT_NOTIFY_PAN_PROFILE_DISCONNECTED:
    {
        rt_bt_lwip_event_handle(bt_lwip_get_default_pan_dev(), RT_BT_PROT_EVT_DISCONNECT);
        break;
    }
    default:
        break;
     }
     return RT_EOK;
 }

 rt_err_t bt_lwip_pan_dev_send_data(struct rt_bt_lwip_pan_dev *bt_dev, void *buff, int len)
 {

    if(bt_dev)
    {
        extern void bt_pan_send_data(void *buff, int len);
        bt_pan_send_data(buff, len);
    }
    return RT_EOK;
 }
#ifdef __cplusplus
}
#endif

#endif
