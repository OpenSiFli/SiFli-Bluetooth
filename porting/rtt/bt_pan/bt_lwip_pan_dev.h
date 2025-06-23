/*
 * SPDX-FileCopyrightText: 2023-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
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
