/**
  ******************************************************************************
  * @file   bt_lwip_pan_dev.h
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

#ifndef __BT_LWIP_DEV_PAN_H__
#define __BT_LWIP_DEV_PAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PAN_INSTANCE_NUM       1

typedef enum
{
    RT_BT_INSTANCE_EVT_INIT_DONE = 0,
    RT_BT_INSTANCE_EVT_CONNECT,
    RT_BT_INSTANCE_EVT_DISCONNECT,
    RT_BT_INSTANCE_EVT_MAX,
} rt_bt_dev_event_t;

struct rt_bt_lwip_pan_dev;

typedef void (*rt_bt_pan_dev_event_handler)(struct rt_bt_lwip_pan_dev *bt_dev, rt_bt_dev_event_t event);

struct rt_bt_dev_ops
{
    rt_err_t (*bt_init)(struct rt_bt_lwip_pan_dev *bt_dev);
    rt_err_t (*bt_set_mac)(struct rt_bt_lwip_pan_dev *bt_dev, rt_uint8_t mac[]);
    rt_err_t (*bt_get_mac)(struct rt_bt_lwip_pan_dev *bt_dev, rt_uint8_t mac[]);
    rt_err_t (*bt_recv)(struct rt_bt_lwip_pan_dev *bt_dev, void *buff, int len);
    rt_err_t (*bt_send)(struct rt_bt_lwip_pan_dev *bt_dev, void *buff, int len);
};

struct rt_bt_lwip_pan_dev
{
    // struct rt_device device;
    void *prot;
    rt_bt_pan_dev_event_handler handler_table[RT_BT_INSTANCE_EVT_MAX][MAX_PAN_INSTANCE_NUM];
    struct rt_bt_dev_ops *ops;
    void *user_data;
};

void bt_lwip_pan_attach_tcpip();
void bt_lwip_pan_detach_tcpip();

void bt_lwip_pan_dev_recv_data(void *buff, int len);
rt_err_t rt_bt_prot_attach_pan_dev(struct rt_bt_lwip_pan_dev *bt_dev);
rt_err_t bt_pan_connect_event_handle(uint16_t event_id);

#ifdef __cplusplus
}
#endif

#endif
