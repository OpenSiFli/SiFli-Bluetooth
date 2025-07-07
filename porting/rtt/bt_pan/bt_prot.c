/*
 * SPDX-FileCopyrightText: 2023-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rthw.h>
#include <rtthread.h>
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



