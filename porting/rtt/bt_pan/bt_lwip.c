/*
 * SPDX-FileCopyrightText: 2023-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rthw.h>
#include <rtthread.h>
#include <stdint.h>
//#include "bnep.h"
#include "bts2_app_pan.h"
#include "bt_lwip.h"
#include "bt_prot.h"
#include "bts2_app_demo.h"

#include "rtdef.h"
#include "bts2_bt.h"

#ifdef RT_USING_LWIP
#include <netif/ethernetif.h>
#include <lwip/netifapi.h>
#include "lwip/pbuf.h"
#include "lwip/timeouts.h"
#ifdef LWIP_USING_DHCPD
    #include <dhcp_server.h>
#endif



#define DBG_TAG "BT.lwip"
//#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#ifndef IPADDR_STRLEN_MAX
    #define IPADDR_STRLEN_MAX    (32)
#endif

struct bt_lwip_prot_des
{
    struct rt_bnep_prot prot;
    struct eth_device eth;
    rt_int8_t connected_flag;
};

extern BTS2S_ETHER_ADDR   bts2_local_ether_addr;
extern void lwip_system_uninit(void);
extern void lwip_sys_init();

void rt_bnep_lwip_event_handle(struct rt_bnep_device_t *bt_dev, int event, int only_flag)
{
    struct bt_lwip_prot_des *lwip_prot = (struct bt_lwip_prot_des *)bt_dev->prot;
    struct eth_device *eth_dev = &lwip_prot->eth;

    switch (event)
    {
    case RT_BT_PROT_EVT_CONNECT:
    {
        LOG_D("event: CONNECT");
        lwip_prot->connected_flag = RT_TRUE;
        if (only_flag)
        {
            LOG_D("sys_timeouts_init");
            sys_timeouts_init();//to restart timer
        }
        // netif_set_default(eth_dev->netif);
        netifapi_netif_common(eth_dev->netif, netif_set_link_up, NULL);
        if (bt_dev->mode == RT_BNEP_PANU)
        {
#ifdef RT_LWIP_DHCP
            // dhcp_start(eth_dev->netif);
#endif
        }
        else if (bt_dev->mode == RT_BNEP_NAP)
        {
            // LOG_D("CONNECT  RT_BNEP_NAP");
#ifdef LWIP_USING_DHCPD
            char netif_name[RT_NAME_MAX];
            rt_memset(netif_name, 0, sizeof(netif_name));
            rt_memcpy(netif_name, lwip_prot->eth.netif->name, sizeof(lwip_prot->eth.netif->name));
            dhcpd_start(netif_name);
#endif
        }
        break;
    }
    case RT_BT_PROT_EVT_DISCONNECT:
    {
        LOG_D("event: DISCONNECT");
        lwip_prot->connected_flag = RT_FALSE;
        netifapi_netif_common(eth_dev->netif, netif_set_link_down, NULL);
        if (bt_dev->mode == RT_BNEP_PANU)
        {
#ifdef RT_LWIP_DHCP
            ip_addr_t ip_addr = { 0 };
            dhcp_stop(eth_dev->netif);
            netif_set_addr(eth_dev->netif, &ip_addr, &ip_addr, &ip_addr);
#endif
        }
        else if (bt_dev->mode == RT_BNEP_NAP)
        {
            // LOG_D("DISCONNECT  RT_BNEP_NAP");
#ifdef LWIP_USING_DHCPD
            char netif_name[RT_NAME_MAX];
            rt_memset(netif_name, 0, sizeof(netif_name));
            rt_memcpy(netif_name, lwip_prot->eth.netif->name, sizeof(lwip_prot->eth.netif->name));
            dhcpd_stop(netif_name);
#endif
        }
        if (only_flag)
        {
            LOG_D("lwip_system_uninit");
            lwip_system_uninit(); // to stop lwip timer
        }
        break;
    }
    default :
    {
        LOG_D("event: UNKNOWN");
        break;
    }
    }
}

BTS2S_ETHER_ADDR bt_pan_get_mac_address()
{
    BTS2S_ETHER_ADDR   local_ether_addr;
    local_ether_addr = bts2_local_ether_addr;
    return local_ether_addr;
}

static rt_err_t rt_bt_lwip_protocol_control(rt_device_t device, int cmd, void *args)
{
    struct eth_device *eth_dev = (struct eth_device *)device;
    struct rt_bnep_device_t  *bt_dev;
    rt_err_t err = RT_EOK;
    BTS2S_ETHER_ADDR   mac_addr;
    rt_uint8_t *address;

    RT_ASSERT(eth_dev != RT_NULL);

    address = (rt_uint8_t *)args;

    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get MAC address */
        bt_dev = eth_dev->parent.user_data;
        mac_addr = bt_pan_get_mac_address();

        address[0] = (mac_addr.w[0] >> 8) & 0xff;
        address[1] = mac_addr.w[0] & 0xff;
        address[2] = (mac_addr.w[1] >> 8) & 0xff;
        address[3] = mac_addr.w[1] & 0xff;
        address[4] = (mac_addr.w[2] >> 8) & 0xff;
        address[5] = mac_addr.w[2] & 0xff;

        LOG_D("address[0] %x\n", address[0]);
        LOG_D("address[1] %x\n", address[1]);
        LOG_D("address[2] %x\n", address[2]);
        LOG_D("address[3] %x\n", address[3]);
        LOG_D("address[4] %x\n", address[4]);
        LOG_D("address[5] %x\n", address[5]);
        break;
    default :
        break;
    }
    return err;
}

static rt_err_t rt_bt_lwip_protocol_recv(struct rt_bnep_device_t *bt_dev, void *buff, int len)
{
    struct eth_device *eth_dev = &((struct bt_lwip_prot_des *)bt_dev->prot)->eth;
    struct pbuf *p = RT_NULL;

    // LOG_D("F:%s L:%d run", __FUNCTION__, __LINE__);

    if (eth_dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    int count = 0;

    while (p == RT_NULL)
    {
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        if (p != RT_NULL)
            break;

        p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
        if (p != RT_NULL)
            break;

        LOG_D("F:%s L:%d wait for pbuf_alloc!", __FUNCTION__, __LINE__);
        rt_thread_delay(1);
        count++;

        //wait for 10ms or give up!!
        if (count >= 10)
        {
            LOG_W("F:%s L:%d pbuf allocate fail!!!", __FUNCTION__, __LINE__);
            return -RT_ENOMEM;
        }
    }

    /*copy data dat -> pbuf*/
    pbuf_take(p, buff, len);
    LOG_D("rt_bt_lwip_protocol_recv netif %p", eth_dev->netif);
    if ((eth_dev->netif->input(p, eth_dev->netif)) != ERR_OK)
    {
        LOG_D("F:%s L:%d IP input error", __FUNCTION__, __LINE__);
        pbuf_free(p);
        p = RT_NULL;
    }

    // LOG_D("F:%s L:%d netif iput success! len:%d", __FUNCTION__, __LINE__, len);
    return RT_EOK;

}

static rt_err_t rt_bt_lwip_protocol_send(rt_device_t device, struct pbuf *p)
{
    struct rt_bnep_device_t *bt_dev = ((struct eth_device *)device)->parent.user_data;

    //LOG_D("F:%s L:%d run", __FUNCTION__, __LINE__);

    rt_uint8_t *frame;

    /* sending data directly */
    if (p->len == p->tot_len)
    {
        // rt_kprintf("enter rt_bt_lwip_protocol_send total ,total len %d\n",p->tot_len);
        frame = (rt_uint8_t *)p->payload;
        rt_bt_prot_send_data(bt_dev, frame, p->tot_len);
        // LOG_D("F:%s L:%d run len:%d", __FUNCTION__, __LINE__, p->tot_len);
        return RT_EOK;
    }

    frame = rt_malloc(p->tot_len);
    if (frame == RT_NULL)
    {
        LOG_E("F:%s L:%d malloc out_buf fail\n", __FUNCTION__, __LINE__);
        return -RT_ENOMEM;
    }
    /*copy pbuf -> data dat*/
    pbuf_copy_partial(p, frame, p->tot_len, 0);
    /* send data */
    //rt_kprintf("enter rt_bt_lwip_protocol_send fragment ,total len %d\n",p->tot_len);
    rt_bt_prot_send_data(bt_dev, frame, p->tot_len);
    // LOG_D("F:%s L:%d run len:%d", __FUNCTION__, __LINE__, p->tot_len);
    rt_free(frame);
    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops bt_lwip_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    rt_bt_lwip_protocol_control
};
#endif

static struct rt_bnep_prot *rt_bt_lwip_protocol_register(struct rt_bnep_prot *prot, struct rt_bnep_device_t *bt_dev)
{
    struct eth_device *eth = RT_NULL;
    char eth_name[4], timer_name[16];
    rt_device_t device = RT_NULL;
    static struct bt_lwip_prot_des *lwip_prot = NULL;
    rt_uint8_t id = 0;
    static rt_bool_t init_ok = RT_FALSE;
    // lwip_sys_init();// to init lwip and lwip timer
    LOG_I("lwip:enter register_eth %d  %d\n", bt_dev, prot);

    if (bt_dev == RT_NULL || prot == RT_NULL)
    {
        return RT_NULL;
    }

    do
    {
        /* find ETH device name */
        eth_name[0] = 'b';
        eth_name[1] = '0' + id++;
        eth_name[2] = '\0';
        device = rt_device_find(eth_name);
    }
    while (device);

    if (id > 9)
    {
        LOG_E("F:%s L:%d not find Empty name", __FUNCTION__, __LINE__, eth_name);
        return RT_NULL;
    }

    lwip_prot = rt_malloc(sizeof(struct bt_lwip_prot_des));

    if (lwip_prot == RT_NULL)
    {
        //shutdown bnep connection????
        LOG_E("F:%s L:%d malloc mem failed", __FUNCTION__, __LINE__);
        return RT_NULL;
    }

    rt_memset(lwip_prot, 0, sizeof(struct bt_lwip_prot_des));
    eth = &lwip_prot->eth;

#ifdef RT_USING_DEVICE_OPS
    eth->parent.ops        = &bt_lwip_ops;
#else
    eth->parent.init       = RT_NULL;
    eth->parent.open       = RT_NULL;
    eth->parent.close      = RT_NULL;
    eth->parent.read       = RT_NULL;
    eth->parent.write      = RT_NULL;
    eth->parent.control    = rt_bt_lwip_protocol_control;
#endif

    eth->parent.user_data  = bt_dev;
    eth->eth_rx     = RT_NULL;
    eth->eth_tx     = rt_bt_lwip_protocol_send;

    /* register ETH device */
    if (eth_device_init(eth, eth_name) != RT_EOK)
    {
        LOG_E("eth device init failed");
        rt_free(lwip_prot);
        lwip_prot = RT_NULL;
        return RT_NULL;
    }
    rt_memcpy(&lwip_prot->prot, prot, sizeof(struct rt_bnep_prot));
    netif_set_up(eth->netif);

    if (!init_ok)
    {
        lwip_system_uninit(); // To stop timer,and Reduce power consumption
        init_ok = RT_TRUE;
    }
    LOG_I("eth device init ok name:%s %p", eth_name, &lwip_prot->prot);
    return &lwip_prot->prot;
}

static void rt_bt_lwip_protocol_unregister(struct rt_bnep_prot *prot, struct rt_bnep_device_t *bt_dev)
{
    struct bt_lwip_prot_des *lwip_prot = (struct bt_lwip_prot_des *)bt_dev->prot;

    LOG_D("F:%s L:%d is run bnep", __FUNCTION__, __LINE__);

    if (lwip_prot == RT_NULL)
    {
        return;
    }

#ifdef LWIP_USING_DHCPD
    {
        char netif_name[RT_NAME_MAX];
        rt_memset(netif_name, 0, sizeof(netif_name));
        rt_memcpy(netif_name, lwip_prot->eth.netif->name, sizeof(lwip_prot->eth.netif->name));
        dhcpd_stop(netif_name);
    }
#endif
    eth_device_deinit(&lwip_prot->eth);
    rt_free(lwip_prot);
}

static struct rt_bt_prot_ops ops =
{
    rt_bt_lwip_protocol_recv,
    rt_bt_lwip_protocol_register,
    rt_bt_lwip_protocol_unregister
};


int rt_bt_lwip_init(void)
{
    static struct rt_bnep_prot prot;
    rt_bnep_prot_event_t event;

    rt_memset(&prot, 0, sizeof(prot));
    rt_strncpy(&prot.name[0], RT_BT_PROT_LWIP, RT_BT_PROT_NAME_LEN);
    prot.ops = &ops;

    if (rt_bt_prot_regisetr(&prot) != RT_EOK)
    {
        LOG_E("F:%s L:%d protocol regisetr failed", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

INIT_PREV_EXPORT(rt_bt_lwip_init);
#endif
