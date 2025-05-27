/**
  ******************************************************************************
  * @file   bt_port.h
  * @author Sifli software development team
  ******************************************************************************
*/
/*
 * @attentionv
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
#ifndef __BT_PROT_H__
#define __BT_PROT_H__

#include "bt_lwip_pan_dev.h"



#ifdef __cplusplus
extern "C" {
#endif

#ifndef RT_BT_PROT_NAME_LEN
#define RT_BT_PROT_NAME_LEN  (8)
#endif



#define RT_BT_PROT_LWIP  ("lwip")

typedef enum
{
    RT_BT_PROT_EVT_INIT_DONE = 0,
    RT_BT_PROT_EVT_CONNECT,
    RT_BT_PROT_EVT_DISCONNECT,
    RT_BT_PROT_EVT_MAX,
} rt_bt_prot_event_t;


struct rt_bt_prot;

struct rt_bt_prot_ops
{
    rt_err_t (*prot_recv)(struct rt_bt_lwip_pan_dev *bt_dev, void *buff, int len);
    struct rt_bt_prot *(*dev_reg_callback)(struct rt_bt_prot *prot, struct rt_bt_lwip_pan_dev *bt_dev);
    void (*dev_unreg_callback)(struct rt_bt_prot *prot, struct rt_bt_lwip_pan_dev *bt_dev);
};


struct rt_bt_prot
{
    char name[RT_BT_PROT_NAME_LEN];
    const struct rt_bt_prot_ops *ops;
};


typedef void (*rt_bt_prot_event_handler)(struct rt_bt_prot *port, struct rt_bt_lwip_pan_dev *bt_dev, int event);

static void rt_bt_prot_event_handle(struct rt_bt_lwip_pan_dev *bt_dev, rt_bt_dev_event_t event);
rt_err_t rt_bt_prot_attach_pan_dev(struct rt_bt_lwip_pan_dev *bt_dev);

rt_err_t rt_bt_prot_detach_pan_dev(struct rt_bt_lwip_pan_dev *bt_dev);
rt_err_t rt_bt_prot_regisetr(struct rt_bt_prot *prot);
//rt_err_t rt_bt_prot_event_register(struct rt_bt_prot *prot, rt_bt_prot_event_t event, rt_bt_prot_event_handler handler);
rt_err_t rt_bt_prot_event_unregister(struct rt_bt_prot *prot, rt_bt_prot_event_t event);
rt_err_t rt_bt_prot_send_data(struct rt_bt_lwip_pan_dev *bt_dev, void *buff, int len);
rt_err_t rt_bt_prot_recv_data(struct rt_bt_lwip_pan_dev *bt_dev, void *buff, int len);

#ifdef __cplusplus
}
#endif

#endif
