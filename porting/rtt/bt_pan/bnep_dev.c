/*
 * SPDX-FileCopyrightText: 2023-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BT_DEV_PAN_H__
#define __BT_DEV_PAN_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <rthw.h>
#include <rtthread.h>
#include "rtdef.h"
#include "bts2_app_inc.h"
#include "bt_prot.h"

#include <rtdbg.h>

rt_err_t bnep_dev_send_data(struct rt_bnep_device_t *bt_dev, void *buff, int len);

struct rt_bnep_device_t bnep_dev[MAX_PAN_INSTANCE_NUM] = {0};

struct rt_bnep_device_ops_t dev_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    bnep_dev_send_data
};
/*-------------------device manager -------------------------------*/
static rt_err_t _init_device(struct rt_bnep_device_t *bt_dev, rt_uint8_t *mac, rt_bnep_mode_t mode)
{
    if (bt_dev == RT_NULL || mac == NULL)
    {
        return -RT_ERROR;
    }
    rt_memcpy(&bt_dev->addr, mac, sizeof(bt_dev->addr));
    bt_dev->mode = mode;
    return -RT_EOK;
}

static struct rt_bnep_device_t *_alloc_device(void)
{
    for (int i = 0; i < MAX_PAN_INSTANCE_NUM; i++)
    {
        if (bnep_dev[i].used == 0)
        {
            bnep_dev[i].used = 1;
            bnep_dev[i].id = i;
            bnep_dev[i].ops = &dev_ops;
            bnep_dev[i].bnep_id = 0xffff;
            rt_memset(&bnep_dev[i].addr, 0, sizeof(bnep_dev[i].addr));
            return &bnep_dev[i];
        }
    }

    return RT_NULL;
}

static rt_err_t _free_device(struct rt_bnep_device_t *bt_dev)
{
    if (bt_dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    for (int i = 0; i < MAX_PAN_INSTANCE_NUM; i++)
    {
        if (&bnep_dev[i] == bt_dev)
        {
            if (bnep_dev[i].used == 1)
            {
                bnep_dev[i].used = 0;
                bnep_dev[i].ops = RT_NULL;
                rt_memset(&bnep_dev[i].addr, 0, sizeof(bnep_dev[i].addr));
                bnep_dev[i].bnep_id = 0xffff;
                return RT_EOK;
            }
            else
            {
                return -RT_ERROR;
            }
        }
    }
    return -RT_ERROR;
}

struct rt_bnep_device_t *_get_device(rt_uint8_t *mac)
{
    if (mac == RT_NULL)
    {
        return NULL;
    }

    for (int i = 0; i < MAX_PAN_INSTANCE_NUM; i++)
    {
        if ((bnep_dev[i].used)
                && (!rt_memcmp(&bnep_dev[i].addr, mac, sizeof(bnep_dev[i].addr))))
        {
            return &bnep_dev[i];
        }
    }
    return NULL;
}

struct rt_bnep_device_t *_get_device_by_bnep_id(rt_uint16_t bnep_id)
{
    if (bnep_id == 0xffff)
    {

        return NULL;
    }

    for (int i = 0; i < MAX_PAN_INSTANCE_NUM; i++)
    {
        if ((bnep_dev[i].used) && (bnep_dev[i].bnep_id == bnep_id))
        {
            return &bnep_dev[i];
        }
    }
    return NULL;
}

rt_uint8_t bnep_dev_get_used_device_count()
{
    rt_uint8_t count = 0;

    for (int i = 0; i < MAX_PAN_INSTANCE_NUM; i++)
    {
        if (bnep_dev[i].used)
        {
            count++;
        }
    }
    return count;
}

rt_bnep_mode_t _get_bnep_mode(rt_uint8_t role)
{
    rt_bnep_mode_t mode = RT_BNEP_NONE;
    switch (role)
    {
    case 0x00:
    {
        mode = RT_BNEP_NONE;
        break;
    }
    case 0x01:
    {
        mode = RT_BNEP_NAP;
        break;
    }
    case 0x02:
    {
        mode = RT_BNEP_GN;
        break;
    }
    case 0x04:
    {
        mode = RT_BNEP_PANU;
        break;
    }
    default:
    {
        mode = RT_BNEP_NONE;
        break;
    }
    }
    return mode;

}

static rt_bool_t _is_valid_device(struct rt_bnep_device_t *dev)
{

    return RT_FALSE;
}

void bt_lwip_bnep_attach_device(struct rt_bnep_device_t *dev)
{
    rt_bt_prot_attach_pan_dev(dev);
}

void bt_lwip_bnep_detach_device(struct rt_bnep_device_t *dev)
{
    rt_bt_prot_detach_pan_dev(dev);
}

void rt_lwip_instance_register_event_handler(struct rt_bnep_device_t *bt_dev, rt_bt_dev_event_t event, rt_bnep_device_event_handler handler)
{
    int i = 0;

    for (i = 0; i < MAX_PAN_INSTANCE_NUM; i++)
    {
        bt_dev->handler_table[i] = handler;
    }
}

void bnep_dev_recv_data(uint16_t bnep_id, void *buff, int len)
{
    struct rt_bnep_device_t *bt_dev = _get_device_by_bnep_id(bnep_id);
    if (bt_dev)
    {
        rt_bt_prot_recv_data(bt_dev, buff, len);
    }
}

void rt_bnep_lwip_event_handle(struct rt_bnep_device_t *bt_dev, int event, int only_flag);

rt_err_t bnep_connect_event_handle(bt_notify_profile_state_info_t *info, uint16_t event_id)
{

    switch (event_id)
    {
    case BT_NOTIFY_PAN_PROFILE_CONNECTED:
    {
        struct rt_bnep_device_t *bt_dev = _get_device((rt_uint8_t *)&info->mac);
        if (!bt_dev)
        {
            bt_dev = _alloc_device();
            if (!bt_dev)
            {
                //err__handle//
                return -RT_ERROR;
            }
            rt_bnep_mode_t mode =  _get_bnep_mode(info->profile_role);
            _init_device(bt_dev, (rt_uint8_t *)&info->mac, mode);
            bt_dev->bnep_id = info->profile_channel;
            bt_lwip_bnep_attach_device(bt_dev);
        }
        if (bt_dev && bt_dev->prot)
        {
            rt_uint8_t count = bnep_dev_get_used_device_count();
            int only = (count > 1) ? 0 : 1;
            rt_bnep_lwip_event_handle(bt_dev, RT_BT_PROT_EVT_CONNECT, only);
        }
        else
        {
            rt_uint8_t count = bnep_dev_get_used_device_count();
            int only = (count > 1) ? 0 : 1;
            if (rt_bt_prot_attach_pan_dev(bt_dev) != RT_EOK)
            {
                LOG_E("F:%s L:%d attach pan dev failed", __FUNCTION__, __LINE__);
                _free_device(bt_dev);
                return -RT_ERROR;
            }
            rt_bnep_lwip_event_handle(bt_dev, RT_BT_PROT_EVT_CONNECT, only);
            RT_ASSERT(bt_dev->prot != NULL);
        }

        break;
    }
    case BT_NOTIFY_PAN_PROFILE_DISCONNECTED:
    {
        struct rt_bnep_device_t *bt_dev = _get_device((rt_uint8_t *)&info->mac);
        if (bt_dev)
        {
            rt_uint8_t count = bnep_dev_get_used_device_count();
            int only = (count > 1) ? 0 : 1;
            rt_bnep_lwip_event_handle(bt_dev, RT_BT_PROT_EVT_DISCONNECT, only);
            rt_bt_prot_detach_pan_dev(bt_dev);
            _free_device(bt_dev);
        }
        break;
    }
    default:
        break;
    }
    return RT_EOK;
}

rt_err_t bnep_dev_send_data(struct rt_bnep_device_t *bt_dev, void *buff, int len)
{
    if (bt_dev)
    {
        void bt_pan_send_data(uint16_t bnep_id, void *buff, int len);
        bt_pan_send_data(bt_dev->bnep_id, buff, len);

    }
    return RT_EOK;
}

#ifdef __cplusplus
}
#endif

#endif
