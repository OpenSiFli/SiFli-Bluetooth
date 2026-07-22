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

static struct rt_bnep_prot *bt_prot[MAX_PAN_INSTANCE_NUM];

static struct rt_bnep_device_t *rt_bnep_prot_find_by_name(const char *name)
{
    rt_device_t device;

    if (name == RT_NULL)
    {
        LOG_E("F:%s L:%d Parameter Wrongful", __FUNCTION__, __LINE__);
        return RT_NULL;
    }
    device = rt_device_find(name);
    if (device == RT_NULL)
    {
        LOG_E("F:%s L:%d not find bnep dev!! name:%s", __FUNCTION__, __LINE__, name);
        return RT_NULL;
    }
    return (struct rt_bnep_device_t *)device;
}

rt_err_t rt_bt_prot_attach_pan_dev(struct rt_bnep_device_t *bt_dev)
{
    rt_uint8_t id;

    if (bt_dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    id = bt_dev->id;
    if ((id >= MAX_PAN_INSTANCE_NUM) ||
            (bt_prot[id] == RT_NULL) ||
            (bt_prot[id]->ops == RT_NULL) ||
            (bt_prot[id]->ops->dev_reg_callback == RT_NULL))
    {
        LOG_E("F:%s L:%d prot invalid, id:%d", __FUNCTION__, __LINE__, id);
        return -RT_ERROR;
    }

    bt_dev->prot = bt_prot[id]->ops->dev_reg_callback(bt_prot[id], bt_dev); /* attach prot */
    if (bt_dev->prot == RT_NULL)
    {
        LOG_E("F:%s L:%d attach prot failed, id:%d", __FUNCTION__, __LINE__, id);
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_bt_prot_detach_pan_dev(struct rt_bnep_device_t *bt_dev)
{
    rt_uint8_t id;

    if (bt_dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    id = bt_dev->id;
    if ((id >= MAX_PAN_INSTANCE_NUM) ||
            (bt_prot[id] == RT_NULL) ||
            (bt_prot[id]->ops == RT_NULL) ||
            (bt_prot[id]->ops->dev_unreg_callback == RT_NULL))
    {
        LOG_E("F:%s L:%d prot invalid, id:%d", __FUNCTION__, __LINE__, id);
        return -RT_ERROR;
    }

    bt_prot[id]->ops->dev_unreg_callback(bt_prot[id], bt_dev); /* detach prot */
    bt_dev->prot = RT_NULL;
    return RT_EOK;
}

rt_err_t rt_bt_prot_regisetr(struct rt_bnep_prot *prot)
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
    for (i = 0; i < MAX_PAN_INSTANCE_NUM; i++)
    {
        if (bt_prot[i] == RT_NULL)
        {
            id = (MAX_PAN_INSTANCE_NUM << 16) | num;
            prot->id = id;
            bt_prot[i] = prot;
            num ++;
            return RT_EOK;
        }
    }

    /* is full */
    LOG_E("F:%s L:%d Space full", __FUNCTION__, __LINE__);
    return -RT_ERROR;
}

rt_err_t rt_bt_prot_event_unregister(struct rt_bnep_prot *prot, rt_bnep_prot_event_t event)
{
    int i;

    if (prot == RT_NULL)
    {
        return -RT_EINVAL;
    }

    return -RT_ERROR;
}

rt_err_t rt_bt_prot_send_data(struct rt_bnep_device_t *bt_dev, void *buff, int len)
{
    if (bt_dev->ops->bt_send != RT_NULL)
    {
        bt_dev->ops->bt_send(bt_dev, buff, len);//to outside pan
        return RT_EOK;
    }
    return -RT_ERROR;
}

rt_err_t rt_bt_prot_recv_data(struct rt_bnep_device_t *bt_dev, void *buff, int len)
{
    struct rt_bnep_prot *prot = bt_dev->prot;

    if (prot != RT_NULL)
    {
        return prot->ops->prot_recv(bt_dev, buff, len);  //to tcp ip
    }
    return -RT_ERROR;
}