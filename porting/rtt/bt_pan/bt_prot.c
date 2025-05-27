/**
  ******************************************************************************
  * @file   bt_port.c
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

#include <rthw.h>
#include <rtthread.h>
#include "lwip/pbuf.h"
#include "rtdef.h"
#include "bts2_bt.h"
#include "bts2_app_pan.h"
#include "bt_prot.h"


#define DBG_TAG "BLUETOOTH.prot"
//#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static struct rt_bt_prot *bt_prot;

rt_err_t rt_bt_prot_attach_pan_dev(struct rt_bt_lwip_pan_dev *bt_dev)
{
    if (bt_prot != NULL && (bt_prot->ops->dev_reg_callback != NULL))
    {
        bt_dev->prot = bt_prot->ops->dev_reg_callback(bt_prot, bt_dev); /* attach prot */
    }
    //just for one time
    return RT_EOK;
}

rt_err_t rt_bt_prot_detach_pan_dev(struct rt_bt_lwip_pan_dev *bt_dev)
{
    if (bt_prot != NULL && (bt_prot->ops->dev_reg_callback != NULL))
    {
        bt_prot->ops->dev_unreg_callback(bt_prot, bt_dev); /* detach prot */
        bt_dev->prot = NULL;
    }
    return RT_EOK;
}

rt_err_t rt_bt_prot_regisetr(struct rt_bt_prot *prot)
{
    int i;
    rt_uint32_t id;
    static rt_uint8_t num;

    /* Parameter checking */
    if ((prot == RT_NULL) ||
            (prot->ops->prot_recv == RT_NULL) ||
            (prot->ops->dev_reg_callback == RT_NULL))
    {
        LOG_E("F:%s L:%d Parameter Wrongful", __FUNCTION__, __LINE__);
        return -RT_EINVAL;
    }

    /* save prot */
    bt_prot = prot;

    return RT_EOK;
}

rt_err_t rt_bt_prot_event_unregister(struct rt_bt_prot *prot, rt_bt_prot_event_t event)
{
    int i;

    if (prot == RT_NULL)
    {
        return -RT_EINVAL;
    }

    return -RT_ERROR;
}

rt_err_t rt_bt_prot_send_data(struct rt_bt_lwip_pan_dev *bt_dev, void *buff, int len)
{
    if (bt_dev->ops->bt_send != RT_NULL)
    {
        bt_dev->ops->bt_send(bt_dev, buff, len);//to outside pan
        return RT_EOK;
    }
    return -RT_ERROR;
}

rt_err_t rt_bt_prot_recv_data(struct rt_bt_lwip_pan_dev *bt_dev, void *buff, int len)
{
    struct rt_bt_prot *prot = bt_dev->prot;

    if (prot != RT_NULL)
    {
        return prot->ops->prot_recv(bt_dev, buff, len);  //to tcp ip
    }
    return -RT_ERROR;
}



