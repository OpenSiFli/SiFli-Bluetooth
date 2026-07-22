/*
 * SPDX-FileCopyrightText: 2023-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BNEP_DEV_PAN_H__
#define __BNEP_DEV_PAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bts2_app_interface_type.h"

#define MAX_PAN_INSTANCE_NUM       2

typedef enum
{
    RT_BT_INSTANCE_EVT_INIT_DONE = 0,
    RT_BT_INSTANCE_EVT_CONNECT,
    RT_BT_INSTANCE_EVT_DISCONNECT,
    RT_BT_INSTANCE_EVT_MAX,
} rt_bt_dev_event_t;

typedef enum
{
    RT_BNEP_NONE,
    RT_BNEP_NAP,
    RT_BNEP_GN,
    RT_BNEP_PANU,
    RT_BNEP_MODE_MAX
} rt_bnep_mode_t;

struct rt_bnep_device_t;

typedef void (*rt_bnep_device_event_handler)(struct rt_bnep_device_t *bt_dev, rt_bt_dev_event_t event);

struct rt_bnep_device_ops_t
{
    rt_err_t (*bt_init)(struct rt_bnep_device_t *bt_dev);
    rt_err_t (*bt_set_mac)(struct rt_bnep_device_t *bt_dev, rt_uint8_t mac[]);
    rt_err_t (*bt_get_mac)(struct rt_bnep_device_t *bt_dev, rt_uint8_t mac[]);
    rt_err_t (*bt_recv)(struct rt_bnep_device_t *bt_dev, void *buff, int len);
    rt_err_t (*bt_send)(struct rt_bnep_device_t *bt_dev, void *buff, int len);
};

struct rt_bnep_device_t
{
    rt_uint8_t used;
    rt_uint8_t id;
    rt_uint8_t addr[6];
    rt_uint16_t bnep_id;
    struct rt_device device;
    rt_bnep_mode_t mode;
    struct rt_mutex lock;
    rt_bnep_device_event_handler handler_table[MAX_PAN_INSTANCE_NUM];
    struct rt_bnep_device_ops_t *ops;
    void *user_data;
    void *prot;
};

int rt_bnep_init(void);
rt_err_t rt_bnep_set_mode(const char *dev_name, rt_bnep_mode_t mode);
rt_bnep_mode_t rt_bnep_get_mode(const char *dev_name);

void bt_lwip_bnep_attach_device(struct rt_bnep_device_t *dev);
void bt_lwip_bnep_detach_device(struct rt_bnep_device_t *dev);
void bnep_dev_recv_data(uint16_t bnep_id, void *buff, int len);

rt_err_t rt_bt_prot_attach_pan_dev(struct rt_bnep_device_t *bt_dev);
rt_err_t bnep_connect_event_handle(bt_notify_profile_state_info_t *info, uint16_t event_id);
rt_uint8_t bnep_lwip_get_used_device_count();
#ifdef __cplusplus
}
#endif

#endif
