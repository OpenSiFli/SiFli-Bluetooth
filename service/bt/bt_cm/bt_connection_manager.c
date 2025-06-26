/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>

#ifdef BSP_BT_CONNECTION_MANAGER
#include "config.h"
#include "bts2_app_inc.h"
#include "bf0_sibles_nvds.h"
#include "bt_connection_manager.h"

#ifdef RT_USING_ULOG
    #include "ulog.h"
#else
    #undef DBG_WARNING
    #undef DBG_ERROR
    #include "rtdbg.h"
#endif


#ifdef RT_USING_BT
    #include "bt_rt_device.h"
#endif



#define BT_CM_MAX_TIMEOUT (3000)
#define BT_CM_SNIFF_INV (798)
#define BT_CM_SNIFF_ATTEMPT (4)
#define BT_CM_SNIFF_TIMEOUT (1)
// TODO: Some time iPhone insist using 240ms, so change to 62.5ms first
#define BT_CM_SNIFF_CHG_TH (100)
#define BT_CM_SNIFF_ENTER_TIME (5) // seconds
#define PAGE_SCAN_INTVL_LOW                     0x800
#define PAGE_SCAN_WINDOW_LOW                    0x12
#define PAGE_SCAN_INTVL_HIGH                    0x200
#define PAGE_SCAN_WINDOW_HIGH                   0x24

#define INQUIRY_SCAN_INTVL_LOW                  0x800
#define INQUIRY_SCAN_WINDOW_LOW                 0x12
#define INQUIRY_SCAN_INTVL_HIGH                 0x200
#define INQUIRY_SCAN_WINDOW_HIGH                0x24



OS_TIMER_DECLAR(g_btcm_timer);

// master and slave profile target
static uint32_t g_bt_cm_mp_tar = BT_CM_DEFAULT_MASTER_BIT;
static uint32_t g_bt_cm_sp_tar = BT_CM_DEFAULT_SLAVE_BIT;

static bt_cm_device_manager_t g_bt_cm_env;
static bt_cm_bonded_dev_t g_bt_bonded_dev;

static uint32_t bt_cm_conn_get_next_profile(bt_cm_dev_acl_info_t *conn, bt_cm_link_type_t link_type);
static uint32_t bt_cm_get_profile_target(bt_cm_link_type_t link_type);
static uint8_t bt_cm_get_reconnect_flag_by_role(bt_cm_link_type_t link_type);
static void bt_fsm_hook_fun(const uint8_t *string, uint8_t state, uint8_t evt);


bt_cm_device_manager_t *bt_cm_get_env()
{
    return &g_bt_cm_env;
}

bt_cm_bonded_dev_t *bt_cm_get_bonded_dev(void)
{
    return &g_bt_bonded_dev;
}

static uint8_t bt_cm_get_link_type_by_profile_role(uint8_t profile, uint8_t role)
{
    uint8_t link_type = BT_LINK_PHONE;
    switch (profile)
    {
    case BT_NOTIFY_HFP_PROFILE:
    {
        if (role == BT_NOTIFY_HFP_AG)
        {
            link_type = BT_LINK_EARPHONE;
        }
        break;
    }
    case BT_NOTIFY_AVRCP_PROFILE:
    {
        if (role == BT_NOTIFY_AVRCP_ROLE_TG)
        {
            link_type = BT_LINK_EARPHONE;
        }
        break;
    }
    case BT_NOTIFY_A2DP_PROFILE:
    {
        if (role == BT_NOTIFY_A2DP_ROLE_SOURCE)
        {
            link_type = BT_LINK_EARPHONE;
        }
        break;
    }
    }
    return link_type;
}

static uint8_t bt_cm_get_link_type_by_cls(uint32_t dev_cls)
{
    uint8_t link_type = BT_LINK_PHONE;

    if (dev_cls & BT_DEVCLS_PHONE)
    {
        return link_type;
    }

    if (dev_cls & (BT_SRVCLS_AUDIO | BT_SRVCLS_RENDER | BT_DEVCLS_AUDIO))
    {
        LOG_I("dev_cls 0x%2x 0x%2x", dev_cls, (BT_SRVCLS_AUDIO | BT_SRVCLS_RENDER | BT_DEVCLS_AUDIO));
        link_type = BT_LINK_EARPHONE;
    }

    return link_type;
}
/*************************************bt connection manager bonded info start *************************************/
int bt_cm_check_bonded_dev(BTS2S_BD_ADDR *bd_addr)
{
    uint8_t i;
    for (i = 0; i < BT_CM_MAX_BOND; i ++)
    {
        if ((g_bt_bonded_dev.info[i].is_use && (bd_eq(&g_bt_bonded_dev.info[i].bd_addr, bd_addr) == TRUE)))
        {
            return i;
        }
    }
    return BT_CM_MAX_BOND;
}

int bt_cm_alloc_bond_dev()
{
    uint8_t i;
    for (i = 0; i < BT_CM_MAX_BOND; i ++)
    {
        if (!g_bt_bonded_dev.info[i].is_use)
        {
            return i;
        }
    }
    return BT_CM_MAX_BOND;
}

void bt_cm_add_bonded_dev(bt_cm_dev_info_t *dev, uint8_t force)
{
    uint8_t idx = bt_cm_check_bonded_dev(&dev->bd_addr);
    if (idx == BT_CM_MAX_BOND)
    {
        idx = bt_cm_alloc_bond_dev();
    }
    if (idx == BT_CM_MAX_BOND && force)
    {
        idx = (g_bt_bonded_dev.last_bond_idx + 1) % BT_CM_MAX_BOND;
        bt_cm_device_manager_t *env = bt_cm_get_env();
        bt_cm_dev_acl_info_t *conn = NULL;
        while (1)
        {
            conn = bt_cm_get_conn_by_addr(env, &g_bt_bonded_dev.info[idx].bd_addr);
            if (conn && conn->state == BT_CM_ACL_STATE_CONNECTED)
            {
                RT_ASSERT(idx != g_bt_bonded_dev.last_bond_idx);
                idx++;
                if (idx == BT_CM_MAX_BOND)
                {
                    idx = 0;
                }
            }
            else
            {
                break;
            }
        }
        sc_unpair_req(bts2_task_get_app_task_id(), &g_bt_bonded_dev.info[idx].bd_addr);
        LOG_I("overwrite the device %x-%x-%x linkey", g_bt_bonded_dev.info[idx].bd_addr.nap, g_bt_bonded_dev.info[idx].bd_addr.uap, g_bt_bonded_dev.info[idx].bd_addr.lap);
    }

    if (idx < BT_CM_MAX_BOND)
    {
        memcpy(&g_bt_bonded_dev.info[idx], dev, sizeof(bt_cm_dev_info_t));
        g_bt_bonded_dev.last_bond_idx = idx;
        g_bt_bonded_dev.info[idx].is_use = 1;
        sifli_nvds_write(SIFLI_NVDS_TYPE_BT_CM, sizeof(bt_cm_bonded_dev_t), (uint8_t *)&g_bt_bonded_dev);
    }
}

static uint8_t bt_cm_get_bond_info_from_flash()
{
    uint8_t ret;

    if (sizeof(g_bt_bonded_dev) > SIFLI_NVDS_KEY_LEN_BT_CM)
        RT_ASSERT(0);

    uint16_t len = sizeof(g_bt_bonded_dev);
    ret = sifli_nvds_read(SIFLI_NVDS_TYPE_BT_CM, &len, (uint8_t *)&g_bt_bonded_dev);

    if (ret != NVDS_OK)
    {
        LOG_E("read bt bonded failed!");
    }

    /*
        LOG_D("read_bt_bond_infor_from_flash: %d\n", ret);

        LOG_D("read_bt_bond_infor_from_flash: %04X:%02X:%06lX\n",
                env->bt_devices.bd_addr.nap,
                env->bt_devices.bd_addr.uap,
                env->bt_devices.bd_addr.lap);
    */
    return ret;
}

void bt_cm_delete_bonded_devs(void)
{
    sc_clean_all_link_key();
    memset(&g_bt_bonded_dev, 0, sizeof(bt_cm_bonded_dev_t));
    sifli_nvds_write(SIFLI_NVDS_TYPE_BT_CM, sizeof(bt_cm_bonded_dev_t), (uint8_t *)&g_bt_bonded_dev);
}

void bt_cm_delete_bonded_devs_and_linkkey(uint8_t *addr)
{
    uint8_t i;
    BTS2S_BD_ADDR     bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)addr, &bd_addr);
    for (i = 0; i < BT_CM_MAX_BOND; i ++)
    {
        if (bd_eq(&g_bt_bonded_dev.info[i].bd_addr, &bd_addr) == TRUE)
        {
            memset(&g_bt_bonded_dev.info[i], 0, sizeof(bt_cm_dev_info_t));
            sifli_nvds_write(SIFLI_NVDS_TYPE_BT_CM, sizeof(bt_cm_bonded_dev_t), (uint8_t *)&g_bt_bonded_dev);
            sc_unpair_req(bts2_task_get_app_task_id(), &bd_addr);
        }
    }
}

void bt_cm_delete_bonded_devs_and_linkkey_by_addr(BTS2S_BD_ADDR *bd_addr)
{
    uint8_t i;
    for (i = 0; i < BT_CM_MAX_BOND; i ++)
    {
        if (bd_eq(&g_bt_bonded_dev.info[i].bd_addr, bd_addr) == TRUE)
        {
            memset(&g_bt_bonded_dev.info[i], 0, sizeof(bt_cm_dev_info_t));
            sifli_nvds_write(SIFLI_NVDS_TYPE_BT_CM, sizeof(bt_cm_bonded_dev_t), (uint8_t *)&g_bt_bonded_dev);
            sc_unpair_req(bts2_task_get_app_task_id(), bd_addr);
        }
    }
}

bt_cm_dev_info_t *bt_cm_get_bonded_dev_by_addr(uint8_t *addr)
{
    uint8_t i;
    BTS2S_BD_ADDR     bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)addr, &bd_addr);
    for (i = 0; i < BT_CM_MAX_BOND; i ++)
    {
        if (bd_eq(&g_bt_bonded_dev.info[i].bd_addr, &bd_addr) == TRUE)
        {
            return &g_bt_bonded_dev.info[i];
        }
    }
    return NULL;
}

uint8_t bt_cm_get_bond_index_by_addr(uint8_t *addr)
{
    uint8_t i;
    BTS2S_BD_ADDR     bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)addr, &bd_addr);
    for (i = 0; i < BT_CM_MAX_BOND; i++)
    {
        if (bd_eq(&g_bt_bonded_dev.info[i].bd_addr, &bd_addr) == TRUE)
        {
            return i;
        }
    }
    return  BT_CM_INVALID_CONN_INDEX;
}

uint8_t bt_cm_get_addr_by_bond_index(uint8_t idx, BTS2S_BD_ADDR *addr)
{
    if (idx >= BT_CM_MAX_BOND)
    {
        return BT_CM_MAX_BOND;
    }

    memcpy(addr, &g_bt_bonded_dev.info[idx].bd_addr, sizeof(BTS2S_BD_ADDR));
    return 0;
}
/*************************************bt connection manager bonded info end *************************************/

/*************************************bt connection manager acl info start *************************************/
bt_cm_dev_acl_info_t *bt_cm_conn_alloc(bt_cm_device_manager_t *env, BTS2S_BD_ADDR *bd_addr, uint8_t link_type)
{
    for (uint8_t i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
    {

        bt_cm_dev_acl_info_t *conn = &env->bt_devices[i];
        memset(conn, 0, sizeof(bt_cm_dev_acl_info_t));
        if (!conn->is_use)
        {
            conn->is_use = 1;
            conn->info.link_type = link_type;
            conn->state = BT_CM_ACL_STATE_CONNECTING;
            conn->sub_state = BT_CM_SUB_STATE_IDLE;
            memcpy(&conn->info.bd_addr, bd_addr, sizeof(BTS2S_BD_ADDR));
            return conn;
        }
    }

    return NULL;
}

static void bt_cm_conn_dealloc(bt_cm_device_manager_t *env, bt_cm_dev_acl_info_t *conn)
{
    uint8_t i;
    for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
    {
        if (&env->bt_devices[i] == conn)
        {
            if (conn->profile_timer_hdl)
            {
                rt_timer_delete(conn->profile_timer_hdl);
                conn->profile_timer_hdl = NULL;
            }
            memset(conn, 0, sizeof(bt_cm_dev_acl_info_t));
            return;
        }
    }

    // If destory wrongly pointer
    RT_ASSERT(0);
}

bt_cm_dev_acl_info_t *bt_cm_get_conn_by_addr(bt_cm_device_manager_t *env, BTS2S_BD_ADDR *bd_addr)
{
    uint8_t i;
    for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
    {
        if (env->bt_devices[i].state != BT_CM_ACL_STATE_DISCONNECTED
                && (bd_eq(bd_addr, &env->bt_devices[i].info.bd_addr) == TRUE))
        {
            return &env->bt_devices[i];
        }
    }

    return NULL;
}

static bt_cm_dev_acl_info_t *bt_cm_get_conn_by_hdl(bt_cm_device_manager_t *env, uint16_t hdl)
{
    uint8_t i;
    for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
    {
        if (env->bt_devices[i].state != BT_CM_ACL_STATE_DISCONNECTED
                && (env->bt_devices[i].conn_hdl == hdl))
        {
            return &env->bt_devices[i];
        }
    }
    return NULL;
}


static uint8_t bt_cm_get_conn_num(bt_cm_device_manager_t *env)
{
    uint8_t i;
    uint8_t n = 0;
    for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
    {
        if (env->bt_devices[i].state != BT_CM_ACL_STATE_DISCONNECTED)
            n++;
    }

    return n;
}
/*************************************bt connection manager acl info end *************************************/

void init_bt_cm()
{
    bt_cm_device_manager_t *env = bt_cm_get_env();
    memset(env, 0, sizeof(bt_cm_device_manager_t));

#ifdef CFG_OPEN_SCAN
    env->close_process = BT_CM_OPENED;
#else
    env->close_process = BT_CM_CLOSED;
#endif

    bt_cm_get_bond_info_from_flash();

}

uint8_t bt_open_bt_request(void)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();
    env->close_process = BT_CM_OPENED;
    gap_open_req();
    gap_wr_scan_enb_req(bts2_task_get_app_task_id(), TRUE, TRUE);
    return 0;
}

uint8_t bt_close_bt_request(void)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();
    uint8_t  i;
    uint8_t  conn_num = 0;

    gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 0, 0);

    env->close_process = BT_CM_CLOSING;

    for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
    {
        if (env->bt_devices[i].state >= BT_CM_ACL_STATE_CONNECTED)
        {
            conn_num++;
            gap_close_req(&env->bt_devices[i].info.bd_addr);
        }
    }

    LOG_I("bt_close_bt_request, close_process%d conn_num %d ", env->close_process, conn_num);

    if (0 == conn_num)
    {
        gap_close_req(NULL);
        env->close_process = BT_CM_CLOSED;
        for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
        {
            env->bt_devices[i].state =  BT_CM_ACL_STATE_DISCONNECTED;
        }
        // Cal AVRCP
        bt_interface_bt_event_notify(BT_NOTIFY_COMMON, BT_NOTIFY_COMMON_CLOSE_COMPLETE, NULL, 0);
    }

    return 0;
}

int bt_open_bt_request_scan(uint8_t scan)
{
    BOOL inquiry_scan = true, page_scan = true;

    bt_cm_device_manager_t *env = bt_cm_get_env();

    env->close_process = BT_CM_OPENED;
    gap_open_req();
    if (scan == 0)
    {
        inquiry_scan    =   true;
        page_scan       =   true;
        LOG_I("Both inquiry_scan and page_scan open!\n");
    }
    else if (scan == 1)
    {
        inquiry_scan    =   true;
        page_scan       =   false;
        LOG_I("Only inquiry_scan  open!\n");
    }
    else if (scan == 2)
    {
        inquiry_scan    =   false;
        page_scan       =   true;
        LOG_I("Only page_scan  open!\n");
    }

    gap_wr_scan_enb_req(bts2_task_get_app_task_id(), inquiry_scan, page_scan);
    return 0;
}

#ifdef BT_AUTO_CONNECT_LAST_DEVICE

static void bt_cm_reconnect_timeout_hdl(void *parameter)
{
    uint32_t time_state = 0;
    if (parameter != NULL)
    {
        bt_cm_dev_acl_info_t *conn = (bt_cm_dev_acl_info_t *)parameter;
        if (conn->profile_timer_hdl)
        {
            rt_timer_delete(conn->profile_timer_hdl);
            conn->profile_timer_hdl = NULL;
        }

        uint32_t profile_bit = bt_cm_conn_get_next_profile(conn, conn->info.link_type);

        if (profile_bit)
        {
            bt_cm_err_t ret = bt_cm_profile_connect(profile_bit, &conn->info.bd_addr, conn->info.link_type);
            LOG_I("bt_cm_reconnect_timeout_hdl profile_bit %d link_type 0x%2x", profile_bit, conn->info.link_type);

            if (ret != BT_CM_ERR_NO_ERR)
            {
                bt_cm_conn_dealloc(bt_cm_get_env(), conn);
                gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 1, 1);
                return;
            }

            // Avoid scan and page
            gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 0, 0);
        }
        else
        {
            bt_cm_conn_dealloc(bt_cm_get_env(), conn);
            gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 1, 1);
        }
    }
}

static int bt_cm_update_profile_bit_mask(bt_cm_dev_acl_info_t *conn, uint32_t profile_bit)
{
    if (conn)
    {
        conn->profiles_bit_mask |= profile_bit;
        uint32_t profile_type = bt_cm_conn_get_next_profile(conn, conn->info.link_type);
        if (profile_type != 0)
        {
            bt_cm_profile_connect(profile_type,  &conn->info.bd_addr, conn->info.link_type);
        }
    }
    return 0;
}

static int bt_hfp_hf_profile_state_hdl(uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    BTS2S_BD_ADDR     bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)&profile_state->mac, &bd_addr);
    bt_cm_device_manager_t *env = bt_cm_get_env();
    bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &bd_addr);

    switch (event_id)
    {
    case BT_NOTIFY_HF_PROFILE_CONNECTED:
    {
        bt_cm_update_profile_bit_mask(conn, BT_CM_HFP);
        break;
    }
    case BT_NOTIFY_HF_PROFILE_DISCONNECTED:
    {
        break;
    }
    default:
        break;
    }
    return 0;
}

static int bt_hfp_ag_profile_state_hdl(uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    BTS2S_BD_ADDR     bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)&profile_state->mac, &bd_addr);
    bt_cm_device_manager_t *env = bt_cm_get_env();
    bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &bd_addr);

    switch (event_id)
    {
    case BT_NOTIFY_AG_PROFILE_CONNECTED:
    {
        bt_cm_update_profile_bit_mask(conn, BT_CM_HFP);
        break;
    }
    case BT_NOTIFY_AG_PROFILE_DISCONNECTED:
    {
        break;
    }
    default:
        break;
    }
    return 0;
}

static int bt_a2dp_profile_state_hdl(uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    BTS2S_BD_ADDR     bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)&profile_state->mac, &bd_addr);
    bt_cm_device_manager_t *env = bt_cm_get_env();
    bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &bd_addr);

    switch (event_id)
    {
    case BT_NOTIFY_A2DP_PROFILE_CONNECTED:
    {
        bt_cm_update_profile_bit_mask(conn, BT_CM_A2DP);
        break;
    }
    case BT_NOTIFY_A2DP_PROFILE_DISCONNECTED:
    {
        break;
    }
    default:
        break;
    }
    return 0;
}

static int bt_avrcp_profile_state_hdl(uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    BTS2S_BD_ADDR     bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)&profile_state->mac, &bd_addr);
    bt_cm_device_manager_t *env = bt_cm_get_env();
    bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &bd_addr);

    switch (event_id)
    {
    case BT_NOTIFY_AVRCP_PROFILE_CONNECTED:
    {
        bt_cm_update_profile_bit_mask(conn, BT_CM_AVRCP);
        break;
    }
    case BT_NOTIFY_AVRCP_PROFILE_DISCONNECTED:
    {
        break;
    }
    default:
        break;
    }
    return 0;
}

static int bt_hid_profile_state_hdl(uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    BTS2S_BD_ADDR     bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)&profile_state->mac, &bd_addr);
    bt_cm_device_manager_t *env = bt_cm_get_env();
    bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &bd_addr);

    switch (event_id)
    {
    case BT_NOTIFY_HID_PROFILE_CONNECTED:
    {
        bt_cm_update_profile_bit_mask(conn, BT_CM_HID);
        break;
    }
    case BT_NOTIFY_HID_PROFILE_DISCONNECTED:
    {
        break;
    }
    default:
        break;
    }
    return 0;
}

static int bt_pan_profile_state_hdl(uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    BTS2S_BD_ADDR     bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)&profile_state->mac, &bd_addr);
    bt_cm_device_manager_t *env = bt_cm_get_env();
    bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &bd_addr);

    switch (event_id)
    {
    case BT_NOTIFY_PAN_PROFILE_CONNECTED:
    {
        bt_cm_update_profile_bit_mask(conn, BT_CM_PAN);
        break;
    }
    case BT_NOTIFY_PAN_PROFILE_DISCONNECTED:
    {
        break;
    }
    default:
        break;
    }
    return 0;
}

static int bt_spp_profile_state_hdl(uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    // BTS2S_BD_ADDR     bd_addr;
    // bt_addr_convert_to_bts((bd_addr_t *)&profile_state->mac, &bd_addr);

    // switch (event_id)
    // {
    // case BT_NOTIFY_SPP_PROFILE_CONNECTED:
    // {
    //     break;
    // }
    // case BT_NOTIFY_SPP_PROFILE_DISCONNECTED:
    // {
    //     break;
    // }
    // default:
    //     break;
    // }
    return 0;
}

static int bt_gatt_profile_state_hdl(uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    // BTS2S_BD_ADDR     bd_addr;
    // bt_addr_convert_to_bts((bd_addr_t *)&profile_state->mac, &bd_addr);

    // switch (event_id)
    // {
    // case BT_NOTIFY_GATT_PROFILE_CONNECTED:
    // {
    //     break;
    // }
    // case BT_NOTIFY_GATT_PROFILE_DISCONNECTED:
    // {
    //     break;
    // }
    // default:
    //     break;
    // }
    return 0;
}

static int bt_pabp_profile_state_hdl(uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    // BTS2S_BD_ADDR     bd_addr;
    // bt_addr_convert_to_bts((bd_addr_t *)&profile_state->mac, &bd_addr);
    // bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env,&bd_addr);

    // switch (event_id)
    // {
    // case BT_NOTIFY_PBAP_PROFILE_CONNECTED:
    // {
    //     if (conn)
    //     {
    //         conn->profiles_bit_mask  |= BT_CM_A2DP;
    //     }
    //     break;
    // }
    // case BT_NOTIFY_PBAP_PROFILE_DISCONNECTED:
    // {
    //     break;
    // }
    // default:
    //     break;
    // }
    return 0;
}
#endif

void bt_profile_update_connection_state(uint16_t type, uint16_t event_id, bt_notify_profile_state_info_t *profile_state)
{
    bt_interface_bt_event_notify(type, event_id, profile_state, sizeof(bt_notify_profile_state_info_t));
#ifdef BT_AUTO_CONNECT_LAST_DEVICE
    switch (type)
    {
    case BT_NOTIFY_HFP_HF:
    {
        bt_hfp_hf_profile_state_hdl(event_id, profile_state);
        break;
    }
    case BT_NOTIFY_HFP_AG:
    {
        bt_hfp_ag_profile_state_hdl(event_id, profile_state);
        break;
    }
    case BT_NOTIFY_A2DP:
    {
        bt_a2dp_profile_state_hdl(event_id, profile_state);
        break;
    }
    case BT_NOTIFY_AVRCP:
    {
        bt_avrcp_profile_state_hdl(event_id, profile_state);
        break;
    }
    case BT_NOTIFY_HID:
    {
        bt_hid_profile_state_hdl(event_id, profile_state);
        break;
    }
    case BT_NOTIFY_PAN:
    {
        bt_pan_profile_state_hdl(event_id, profile_state);
        break;
    }
    case BT_NOTIFY_SPP:
    {
        bt_spp_profile_state_hdl(event_id, profile_state);
        break;
    }
    case BT_NOTIFY_GATT:
    {
        bt_gatt_profile_state_hdl(event_id, profile_state);
        break;
    }
    case BT_NOTIFY_PBAP:
    {
        bt_pabp_profile_state_hdl(event_id, profile_state);
        break;
    }
    default:
        break;
    }
#endif
}

void bt_close_bt_request_complete_check(bt_cm_device_manager_t *env, bt_cm_dev_acl_info_t *conn)
{
    uint8_t ret = 0;
    if (conn != NULL)
    {
        if (conn->profile_timer_hdl)
        {
            rt_timer_stop(conn->profile_timer_hdl);
            rt_timer_delete(conn->profile_timer_hdl);
            conn->profile_timer_hdl = NULL;
        }
        bt_cm_conn_dealloc(env, conn);
    }

    // double check:Avoid scan and page
    gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 0, 0);
    if (0 == bt_cm_get_conn_num(env))
    {

        if (BT_CM_OPENED != env->close_process)
        {
            bt_interface_bt_event_notify(BT_NOTIFY_COMMON, BT_NOTIFY_COMMON_CLOSE_COMPLETE, NULL, 0);
        }

        env->close_process = BT_CM_CLOSED;
    }

    LOG_I("bt_close_bt_request_complete_check, close_process%d", env->close_process);
}


void bt_cm_close_boundary_condition(bt_cm_device_manager_t *env, bt_cm_dev_acl_info_t *conn, BTS2S_DM_EN_ACL_OPENED_IND *ind)
{
    LOG_I("bt_cm_close_protect, close_process%d st %d conn %d", env->close_process, ind->st, conn);

    if (ind->st == HCI_SUCC)
    {
        if (conn == NULL)
        {
            gap_disconnect_req(&ind->bd);
        }
        else
        {
            bt_cm_conn_dealloc(env, conn);
        }
    }

    gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 0, 0);
}

void bt_cm_change_page_activity(uint8_t is_high)
{
    uint16_t interval = is_high ? PAGE_SCAN_INTVL_HIGH : PAGE_SCAN_INTVL_LOW;
    uint16_t window = is_high ? PAGE_SCAN_WINDOW_HIGH : PAGE_SCAN_WINDOW_LOW;
    hcia_wr_pagescan_activity(interval, window, NULL);
}

void bt_cm_change_inquiryscan_activity(uint8_t is_high)
{
    uint16_t interval = is_high ? INQUIRY_SCAN_INTVL_HIGH : INQUIRY_SCAN_INTVL_LOW;
    uint16_t window = is_high ? INQUIRY_SCAN_WINDOW_HIGH : INQUIRY_SCAN_WINDOW_LOW;
    hcia_wr_inquiryscan_activity(interval, PAGE_SCAN_WINDOW, NULL);
}

static void bt_cm_app_init_ready_hdl(void *msg)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();

#if defined(BT_AUTO_CONNECT_LAST_DEVICE)
    bt_cm_bonded_dev_t *bonded_list = bt_cm_get_bonded_dev();
    uint8_t idx = bonded_list->last_bond_idx;

    if (idx < BT_CM_MAX_BOND)
    {
        bt_cm_dev_info_t *device_info =  bonded_list->info + idx;
        uint8_t link_type1 = device_info->link_type;
        LOG_I("bt_cm_app_init_ready_hdl ,idx %d link_type  %d", idx, device_info->link_type);

        if (!bd_is_empty(&device_info->bd_addr))
        {
            bt_cm_connect_req(&device_info->bd_addr, device_info->link_type);
        }
        else
        {
            gap_wr_scan_enb_req(bts2_task_get_app_task_id(), TRUE, TRUE);
        }
    }
    else
#endif // BT_AUTO_CONNECT_LAST_DEVICE
        if ((BT_CM_OPENED == env->close_process) && (bt_cm_get_conn_num(env) < BT_CM_DEVICE_MAX_CONN))
        {
            gap_wr_scan_enb_req(bts2_task_get_app_task_id(), TRUE, TRUE);
        }

}

int bt_cm_gap_event_handler(uint16_t event_id, uint8_t *msg)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();

    switch (event_id)
    {
    // Using RD LOCAL NAME CFM as app init completed
    case BTS2MU_GAP_RD_LOCAL_NAME_CFM:
    {
        LOG_I("BTS2MU_GAP_RD_LOCAL_NAME_CFM");
        bt_system_mask_clear(BT_RESET_MASK_BT);
        bt_fsm_hook_set(bt_fsm_hook_fun);
        bt_cm_app_init_ready_hdl(NULL);
        break;
    }
    case BTS2MU_GAP_MODE_CHANGED_IND:
    {
        BTS2MU_GAP_MODE_CHANGED_IND_t *ind = (BTS2MU_GAP_MODE_CHANGED_IND_t *)msg;
        uint8_t mod_str[3][7] = {"Active", "Hold", "Sniff"};
        bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &ind->bd);

        if (conn)
        {
            if (conn->link_status_update && ind->st != HCI_SUCC)
            {
                LOG_E("Adjust sniff failed");
                conn->link_status_update = 0;
            }

            if (ind->mode == ACT_MODE && conn->link_status_update)
            {
                conn->link_status_update = 0;
                hcia_sniff_mode(&ind->bd, BT_CM_SNIFF_INV, BT_CM_SNIFF_INV, BT_CM_SNIFF_ATTEMPT, BT_CM_SNIFF_TIMEOUT, NULL);
            }

            if (ind->mode == SNIFF_MODE && ind->st == HCI_SUCC)
            {
                if (ind->interval < BT_CM_SNIFF_CHG_TH)
                {
                    conn->link_status_update = 1;
                    hcia_exit_sniff_mode(&ind->bd, NULL);
                }
            }

            conn->link_mode = ind->mode;
            conn->link_interval = 0.0;

            if (ind->mode > PARK_MODE)
            {
                LOG_W("abnormal mode %d", ind->mode);
            }
            else
            {
                conn->link_interval = (float)ind->interval * 5 / 8;
                LOG_D("%s mode st: %d, inv: %.2f", mod_str[ind->mode], ind->st, conn->link_interval);
            }
        }
        break;
    }
    case BTS2MU_GAP_KEYMISSING_IND:
    {
        BTS2S_GAP_KEYMISSING_IND *ind;
        ind = (BTS2S_GAP_KEYMISSING_IND *)msg;
        bt_cm_delete_bonded_devs_and_linkkey_by_addr(&ind->bd);
        break;
    }
    default:
        break;
    }

    return 0;
}

static void bt_cm_hci_connect_request_event_hdl(BTS2S_DM_ACL_OPEN_IND *ind)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();

    uint8_t link_type = bt_cm_get_link_type_by_cls(ind->dev_cls);

    if (bt_is_auto_request_connect())
    {
        bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &ind->bd);

        if (conn == NULL)
        {
            conn = bt_cm_conn_alloc(env, &ind->bd, link_type);
            if (conn == NULL)
            {
                hcia_send_rej_conn_request(&ind->bd, HCI_ERR_REJ_BY_RMT_NO_RES);
                return;
            }
        }

        hcia_send_acpt_conn_request(&ind->bd, BT_LINK_SLAVE);
    }

    bt_notify_device_conn_request_t conn_request;
    bt_addr_convert(&ind->bd, conn_request.mac.addr);
    conn_request.acl_dir = BT_CM_LINK_INCOMING;
    conn_request.dev_cls = ind->dev_cls;
    conn_request.link_type = link_type;
    bt_interface_bt_event_notify(BT_NOTIFY_COMMON, BT_NOTIFY_COMMON_ACL_CONNECT_IND, &conn_request, sizeof(bt_notify_device_conn_request_t));
}

static void bt_cm_hci_acl_connect_complete_event_hdl(BTS2S_DM_EN_ACL_OPENED_IND *ind)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();
    bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &ind->bd);

    if (conn && ind->st == HCI_SUCC)
    {
        conn->conn_hdl = ind->phdl;
        conn->info.dev_cls = ind->dev_cls;
        conn->link_dir = ind->incoming;
        conn->state = BT_CM_ACL_STATE_CONNECTED;
        conn->info.is_reconn = bt_cm_get_reconnect_flag_by_role(conn->info.link_type);
        hcia_wr_lp_settings_keep_sniff_interval(&ind->bd, HCI_LINK_POLICY_NO_CHANGE, BT_CM_SNIFF_ENTER_TIME,
                                                BT_CM_SNIFF_INV, BT_CM_SNIFF_INV, BT_CM_SNIFF_ATTEMPT, BT_CM_SNIFF_TIMEOUT, NULL);
    }

    else if (conn && (ind->st != HCI_SUCC || (BT_CM_OPENED != env->close_process)))
    {
        bt_cm_conn_dealloc(env, conn);
    }

    if ((bt_cm_get_conn_num(env) + 1) > BT_CM_DEVICE_MAX_CONN)
    {
        gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 0, 0);
    }
    else
    {
        gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 1, 1);
    }

    bt_notify_device_acl_conn_info_t acl_info;
    bt_addr_convert(&ind->bd, acl_info.mac.addr);
    acl_info.res = ind->st;
    acl_info.acl_dir = ind->incoming;
    acl_info.dev_cls = ind->dev_cls;
    acl_info.handle = ind->phdl;
    bt_interface_bt_event_notify(BT_NOTIFY_COMMON, BT_NOTIFY_COMMON_ACL_CONNECTED, &acl_info, sizeof(bt_notify_device_acl_conn_info_t));
}

static void bt_cm_hci_acl_disconnect_complete_event_hdl(BTS2S_DM_ACL_DISC_IND *ind)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();
    bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_hdl(env, ind->hdl);

    bt_notify_device_base_info_t device_info;
    bt_addr_convert(&ind->cur_bd, device_info.mac.addr);
    device_info.res = ind->reason;
    device_info.handle = ind->hdl;
    bt_interface_bt_event_notify(BT_NOTIFY_COMMON, BT_NOTIFY_COMMON_ACL_DISCONNECTED, &device_info, sizeof(bt_notify_device_base_info_t));

#ifdef CFG_HID
    hid_reset_req();
#endif

#ifdef CFG_AV
    a2dp_reset_req();
#endif

#ifdef CFG_AVRCP
    avrcp_reset_req();
#endif

    //close bt branch
    if (BT_CM_CLOSING == env->close_process)
    {
        bt_close_bt_request_complete_check(env, conn);
        return;
    }

    // Enable inquiry and page scan
    if (BT_CM_OPENED == env->close_process)
    {
        gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 1, 1);
    }

    if (conn != NULL)
    {
        bt_cm_link_type_t link_type = conn->info.link_type;
        uint8_t is_reconn = conn->info.is_reconn;
        bt_cm_conn_dealloc(env, conn);
#ifdef BT_AUTO_CONNECT_LAST_DEVICE
        if (is_reconn && ind->reason == HCI_ERR_CONN_TIMEOUT)
        {

            // Try re-connect
            conn = bt_cm_conn_alloc(env, &ind->cur_bd, link_type);
            conn->profile_timer_hdl = rt_timer_create("btcm_con", bt_cm_reconnect_timeout_hdl, conn,
                                      rt_tick_from_millisecond(BT_CM_MAX_TIMEOUT), RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);

            if (conn->profile_timer_hdl)
                rt_timer_start(conn->profile_timer_hdl);
        }
#endif

    }
}

int bt_cm_hci_event_handler(uint16_t event_id, uint8_t *msg)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();
    switch (event_id)
    {
    // acl create connect request to init event
    case DM_ACL_OPEN_IND:
    {
        BTS2S_DM_ACL_OPEN_IND *ind = (BTS2S_DM_ACL_OPEN_IND *)msg;
        bt_cm_hci_connect_request_event_hdl(ind);
        LOG_I("BT_NOTIFY_COMMON_ACL_CONNECT_IND");
        break;
    }
    case DM_EN_ACL_OPENED_IND:
    {
        BTS2S_DM_EN_ACL_OPENED_IND *ind = (BTS2S_DM_EN_ACL_OPENED_IND *)msg;
        LOG_I("link connected COD:%d Incoming:%d hdl %d res %d\r\n", ind->dev_cls, ind->incoming, ind->phdl, ind->st);
        LOG_I("bd addr %x-%x-%x\r\n", ind->bd.nap, ind->bd.uap, ind->bd.lap);
        bt_cm_hci_acl_connect_complete_event_hdl(ind);
        break;
    }
    case DM_ACL_DISC_IND:
    {
        BTS2S_DM_ACL_DISC_IND *ind = (BTS2S_DM_ACL_DISC_IND *)msg;
        LOG_I("link dis-connected %x %d process:%d\r\n", ind->hdl, ind->reason, env->close_process);
        LOG_I("bd addr %x-%x-%x\r\n", ind->cur_bd.nap, ind->cur_bd.uap, ind->cur_bd.lap);
        bt_cm_hci_acl_disconnect_complete_event_hdl(ind);
        break;
    }
    case DM_ACL_CANCEL_CONN_STATUS:
    {
        BTS2S_DM_CMD_STATUS *ind = (BTS2S_DM_CMD_STATUS *)msg;
        LOG_I("DM_ACL_CANCEL_CONN_STATUS");
        break;
    }
    case DM_ACL_DIS_CONN_STATUS:
    {
        BTS2S_DM_CMD_STATUS *ind = (BTS2S_DM_CMD_STATUS *)msg;
        LOG_I("DM_ACL_DIS_CONN_STATUS");
        break;
    }
    case DM_ACL_CREATE_CONN_STATUS:
    {
        BTS2S_DM_CMD_STATUS *ind = (BTS2S_DM_CMD_STATUS *)msg;
        LOG_I("DM_ACL_CREATE_CONN_STATUS");
        break;
    }
    case DM_ACL_ACPT_CONN_STATUS:
    {
        LOG_I("DM_ACL_ACPT_CONN_STATUS");
        break;
    }
    case DM_ACL_REJ_CONN_STATUS:
    {
        BTS2S_DM_CMD_STATUS *ind = (BTS2S_DM_CMD_STATUS *)msg;
        LOG_I("DM_ACL_REJ_CONN_STATUS");
        break;
    }
    case DM_HCI_CREATE_CONN_ESC_COMP:
    {
        LOG_D("DM_HCI_CREATE_CONN_ESC_COMP");
        break;
    }
    default:
        break;
    }
    return 0;

}

int bt_cm_sc_event_handler(uint16_t event_id, uint8_t *msg)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();
    switch (event_id)
    {
    case BTS2MU_SC_RD_PAIRED_DEV_KEY_CFM:
    {
        BTS2S_SC_RD_PAIRED_DEV_KEY_CFM *ind = (BTS2S_SC_RD_PAIRED_DEV_KEY_CFM *)msg;
        LOG_I("Get link key (%x-%x-%x), is_valid(%d):", ind->bd.nap, ind->bd.uap, ind->bd.lap, ind->res == 0);
        if (ind->res == 0)
        {
            LOG_HEX("Key :", 16, ind->link_key, LINK_KEY_SIZE);
        }
        break;
    }
    case BTS2MU_SC_PAIR_IND:
    {
        BTS2S_SC_PAIR_IND *ind;
        ind = (BTS2S_SC_PAIR_IND *)msg;
        bt_cm_device_manager_t *env = bt_cm_get_env();
        if (ind->res == 0)
        {
            LOG_I("BTS2S_SC_PAIR_IND cod:0x%2x", ind->cod);
            bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &ind->bd);
            bt_cm_add_bonded_dev(&conn->info, 1);
            //to add bonded device
        }
        break;
    }
    case BTS2MU_SC_PAIR_CFM:
    {
        break;
    }
    case BTS2MU_SC_PAIRED_DEV_KEY_DELETE_CFM:
    {
        BTS2S_SC_PAIRED_DEV_DELETE_KEY_CFM  *ind = (BTS2S_SC_PAIRED_DEV_DELETE_KEY_CFM *) msg;
        USER_TRACE("BTS2MU_SC_PAIRED_DEV_KEY_DELETE_CFM");
        bt_cm_delete_bonded_devs_and_linkkey_by_addr(&ind->bd);
        break;
    }
    default:
        break;
    }

    return 0;
}

int bt_cm_event_hdl(U16 type, U16 event_id, uint8_t *msg, uint32_t context)
{
    if (type == BTS2M_HCI_CMD)
    {
        bt_cm_hci_event_handler(event_id, msg);
    }
    else if (type == BTS2M_GAP)
    {
        bt_cm_gap_event_handler(event_id, msg);
    }
    else if (type == BTS2M_SC)
    {
        bt_cm_sc_event_handler(event_id, msg);
    }
    return 0;
}
BT_EVENT_REGISTER(bt_cm_event_hdl, NULL);


/*************************************bt connection manager api start *************************************/
static uint8_t bt_cm_conn_check_profile_completed(uint32_t profile_bit,  bt_cm_link_type_t link_type)
{
    uint32_t target = bt_cm_get_profile_target(link_type);
    uint32_t left = target ^ (profile_bit & target);
    return left != 0 ? 0 : 1;
}

uint32_t bt_cm_filter_profile(uint32_t profile)
{
#if !defined(CFG_HFP_HF)&&!defined(CFG_HFP_AG)
    profile &= ~BT_CM_HFP;
#endif

#ifndef CFG_AVRCP
    profile &= ~BT_CM_AVRCP;
#endif

#if !defined(CFG_AV_SRC)&&!defined(CFG_AV_SNK)
    profile &= ~BT_CM_A2DP;
#endif

#ifndef CFG_HID
    profile &= ~BT_CM_HID;
#endif
#ifndef CFG_PAN
    profile &= ~BT_CM_PAN;
#endif

    return profile;
}

static uint32_t bt_cm_conn_get_next_profile(bt_cm_dev_acl_info_t *conn, bt_cm_link_type_t link_type)
{
    uint32_t profile_bit = 0;

    if (conn)
    {

        uint32_t target = bt_cm_get_profile_target(link_type);
        target = bt_cm_filter_profile(target);
        uint32_t left = target ^ (conn->profiles_bit_mask & target);
        uint32_t i;
        for (i = 0; i < 32; i++)
        {
            if (left & (1 << i))
                break;
        }
        if (i < 32)
            profile_bit = 1 << i;
    }

    return profile_bit;
}


static uint32_t bt_cm_get_profile_target(bt_cm_link_type_t link_type)
{
    if (link_type == BT_LINK_EARPHONE)
    {
        return g_bt_cm_mp_tar;
    }
    else if (link_type == BT_LINK_PHONE)
    {
        return g_bt_cm_sp_tar;
    }
    else return 0;

}

void bt_cm_set_profile_target(uint32_t setProfile, bt_cm_link_type_t link_type, uint8_t addFlag)
{
    if ((BT_LINK_EARPHONE == link_type) && (1 == addFlag))
    {
        g_bt_cm_mp_tar |= setProfile;
    }
    else if ((BT_LINK_EARPHONE == link_type) && (0 == addFlag))
    {
        g_bt_cm_mp_tar = setProfile;
    }
    else if ((BT_LINK_PHONE == link_type) && (1 == addFlag))
    {
        g_bt_cm_sp_tar |= setProfile;
    }
    else if ((BT_LINK_PHONE == link_type) && (0 == addFlag))
    {
        g_bt_cm_sp_tar = setProfile;
    }
}

static uint8_t bt_cm_get_reconnect_flag_by_role(bt_cm_link_type_t link_type)
{
    uint8_t reconn_flag = 0;
#ifdef BT_AUTO_CONNECT_LAST_DEVICE
    if (link_type == BT_LINK_PHONE)
        reconn_flag = 1;
#endif

    return reconn_flag;
}

bt_cm_err_t bt_cm_set_target_profiles_by_role(uint32_t profile_bits, bt_cm_link_type_t link_type)
{
    bt_cm_err_t ret = BT_CM_ERR_NO_ERR;
    if (link_type == BT_LINK_EARPHONE)
        g_bt_cm_mp_tar = profile_bits;
    else if (link_type == BT_LINK_PHONE)
        g_bt_cm_sp_tar = profile_bits;
    else
        ret = BT_CM_ERR_INVALID_PARA;

    return ret;
}

bt_err_t bt_interface_profile_connect_request(unsigned char *mac, uint8_t profile, uint8_t role)
{
    bt_err_t err = BT_EOK;
    BTS2S_BD_ADDR bd_addr;

    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);

    if (bd_is_empty(&bd_addr))
    {
        USER_TRACE(">> address invalid\n");
        return BT_ERROR_INPARAM;
    }

    bt_cm_device_manager_t *env = bt_cm_get_env();
    bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, &bd_addr);

    if (conn == NULL)
    {
        uint8_t link_type = bt_cm_get_link_type_by_profile_role(profile, role);
        conn = bt_cm_conn_alloc(env, &bd_addr, link_type);
        if (conn == NULL)
        {
            return BT_ERROR_OUT_OF_MEMORY;
        }
    }

    switch (profile)
    {
    case BT_NOTIFY_HFP_PROFILE:
    {
        if (role == BT_NOTIFY_HFP_ROLE_HF)
        {
            // Avoid scan and page
#ifdef CFG_HFP_HF
            err = bt_hfp_hf_connect_request(&bd_addr);
#endif
        }
        else if (role == BT_NOTIFY_HFP_ROLE_AG)
        {
#ifdef CFG_HFP_AG
            err = bt_hfp_ag_connect_request(&bd_addr);
#endif
        }
        break;
    }
#ifdef CFG_AVRCP
    case BT_NOTIFY_AVRCP_PROFILE:
    {
        if (role == BT_NOTIFY_AVRCP_ROLE_CT)
        {
            err = bt_avrcp_controller_connect_request(&bd_addr);
        }
        else if (role == BT_NOTIFY_AVRCP_ROLE_TG)
        {
            err = bt_avrcp_target_connect_request(&bd_addr);
        }
        break;
    }
#endif
#ifdef CFG_AV
    case BT_NOTIFY_A2DP_PROFILE:
    {
        if (role == BT_NOTIFY_A2DP_ROLE_SOURCE)
        {
#ifdef CFG_AV_SRC
            err = bt_a2dp_sink_connect_request(&bd_addr);
#else
            LOG_I("WARRING,not enabled AV source");
#endif
        }
        else if (role == BT_NOTIFY_A2DP_ROLE_SINK)
        {
#ifdef CFG_AV_SNK
            err = bt_a2dp_source_connect_request(&bd_addr);
#else
            LOG_I("WARRING,not enabled AV sink");
#endif
        }
        break;
    }
#endif
#ifdef BT_FINSH_PAN
    case BT_NOTIFY_PAN_PROFILE:
    {
        err = bt_pan_connect_request(&bd_addr);
        break;
    }
#endif
#ifdef CFG_HID
    case BT_NOTIFY_HID_PROFILE:
    {
        bt_hid_connect_requset(&bd_addr);
        break;
    }
#endif
#ifdef CFG_BR_GATT_SRV
    case BT_NOTIFY_BT_GATT_PROFILE:
    {
        bt_gatt_conn_req(&bd_addr);
        break;
    }
#endif
#ifdef CFG_PBAP_CLT
    case BT_NOTIFY_PBAP_PROFILE:
    {
        err = bt_pbap_client_connect_request(&bd_addr, FALSE);
        break;
    }
#endif
    default:
        return BT_ERROR_UNSUPPORTED;
    }
    return err;
}

bt_err_t bt_interface_acpt_connect_req(unsigned char *mac, uint8_t link_role, uint8_t link_type)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();
    BTS2S_BD_ADDR bd_addr;
    bt_addr_convert_to_bts((bd_addr_t *)mac, &bd_addr);
    bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env,  &bd_addr);
    if (conn == NULL)
    {
        conn = bt_cm_conn_alloc(env,  &bd_addr, link_type);
        if (conn == NULL)
        {
            hcia_send_rej_conn_request(&bd_addr, HCI_ERR_REJ_BY_RMT_NO_RES);
            return BT_ERROR_OUT_OF_MEMORY;
        }
        hcia_send_acpt_conn_request(&bd_addr, link_role);
    }
    return BT_EOK;
}

bt_cm_err_t bt_cm_profile_connect(uint32_t profile_bit, BTS2S_BD_ADDR *bd_addr, bt_cm_link_type_t link_type)
{
    bt_cm_err_t err = BT_CM_ERR_NO_ERR;

#ifdef BT_AUTO_CONNECT_LAST_DEVICE
    LOG_I("profile %d connect", profile_bit);

    uint8_t role = 0xff;
    uint8_t profile = 0xff;

    switch (profile_bit)
    {
    case BT_CM_HFP:
    {
        profile = BT_NOTIFY_HFP_PROFILE;

        if (link_type == BT_LINK_PHONE)
        {
            role = BT_NOTIFY_HFP_ROLE_HF;
        }
        else if (link_type == BT_LINK_EARPHONE)
        {
            role = BT_NOTIFY_HFP_ROLE_AG;
        }
        break;
    }
    case BT_CM_A2DP:
    {
        profile = BT_NOTIFY_A2DP_PROFILE;
        if (link_type == BT_LINK_PHONE)
        {
            role = BT_NOTIFY_A2DP_ROLE_SINK;
        }
        else if (link_type == BT_LINK_EARPHONE)
        {
            role = BT_NOTIFY_A2DP_ROLE_SOURCE;
        }
        break;
    }
    case BT_CM_AVRCP:
    {
        profile = BT_NOTIFY_AVRCP_PROFILE;
        if (link_type == BT_LINK_PHONE)
        {
            role = BT_NOTIFY_AVRCP_ROLE_CT;
        }
        else if (link_type == BT_LINK_EARPHONE)
        {
            role = BT_NOTIFY_AVRCP_ROLE_TG;
        }
        break;
    }
    case BT_CM_PAN:
    {
        profile = BT_NOTIFY_PAN_PROFILE;
        break;
    }
    case BT_CM_HID:
    {
        profile = BT_NOTIFY_HID_PROFILE;
        break;
    }
    default:
        break;
    }

    if (profile != 0xff)
    {
        unsigned char addr[6];
        bt_addr_convert(bd_addr, addr);
        err = bt_interface_profile_connect_request(addr, profile, role);
    }

#endif
    return err;
}

// As master
bt_cm_err_t bt_cm_connect_req(BTS2S_BD_ADDR *bd_addr, bt_cm_link_type_t link_type)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();
    bt_cm_err_t err = BT_CM_ERR_GENERAL_ERR;

#ifdef BT_AUTO_CONNECT_LAST_DEVICE
    do
    {
        bt_cm_dev_acl_info_t *conn = bt_cm_get_conn_by_addr(env, bd_addr);
        if (conn)
        {
            err = BT_CM_ERR_CONN_EXISTED;
            break;
        }

        conn = bt_cm_conn_alloc(env, bd_addr, link_type);
        if (conn == NULL)
        {
            return BT_CM_ERR_RESOURCE_NOT_ENOUGH;
        }

        uint32_t profile_bit = bt_cm_conn_get_next_profile(conn, link_type);

        if (profile_bit)
        {
            bt_cm_err_t ret = bt_cm_profile_connect(profile_bit, bd_addr, link_type);
            LOG_I("Reconnect ret %d link_type 0x%2x", ret, link_type);

            if (ret != BT_CM_ERR_NO_ERR)
            {
                err = BT_CM_ERR_INVALID_PARA;
                bt_cm_conn_dealloc(env, conn);
                gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 1, 1);
                break;
            }

            // Avoid scan and page
            gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 0, 0);
        }
        else
        {
            bt_cm_conn_dealloc(env, conn);
            gap_wr_scan_enb_req(bts2_task_get_app_task_id(), 1, 1);
        }
        err = BT_CM_ERR_NO_ERR;
    }
    while (0);
#endif

    return err;

}

void bt_cm_disconnect_req(void)
{
    bt_cm_device_manager_t *env = bt_cm_get_env();
    uint8_t i;
    for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
    {
        if (env->bt_devices[i].state >= BT_CM_ACL_STATE_CONNECTED)
        {
            gap_disconnect_req(&env->bt_devices[i].info.bd_addr);
        }
    }
}
/*************************************bt connection manager api end *************************************/
static void bt_fsm_hook_fun(const uint8_t *string, uint8_t state, uint8_t evt)
{
    // Not print l2cap recv data to avoid too much data in A2DP streaming scenario.
    if (strcmp((const char *)string, "L2C_CH") == 0 && state == 2 && (evt == 7 || evt == 9))
        return;

    if (strcmp((const char *)string, "L2C_CID") == 0 && state == 5 && evt == 3)
        return;

    if (strcmp((const char *)string, "RFC_MUX") == 0 && state == 6 && evt == 25)
        return;

    if (strcmp((const char *)string, "RFC_DLC") == 0 && state == 4 && (evt == 14 || evt == 28))
        return;

    if (strcmp((const char *)string, "no_module") == 0)
        return;

    LOG_D("fsm: %s st:%d evt:%d", string, state, evt);
}

void bt_cm(uint8_t argc, char **argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "discon") == 0)
        {
            bt_cm_device_manager_t *env = bt_cm_get_env();
            uint8_t i;
            for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
            {
                if (env->bt_devices[i].state >= BT_CM_ACL_STATE_CONNECTED)
                    gap_disconnect_req(&env->bt_devices[i].info.bd_addr);
            }
        }
        else if (strcmp(argv[1], "search") == 0)
        {
            BTS2S_CPL_FILTER inq_filter;

            inq_filter.filter = BTS2_INQ_FILTER_CLEAR;
            inq_filter.dev_mask_cls = BT_DEVCLS_MISC;
            gap_discov_req(bts2_task_get_app_task_id(), atoi(argv[2]), atoi(argv[3]), &inq_filter, TRUE);
        }
        else if (strcmp(argv[1], "conn") == 0)
        {
            BTS2S_BD_ADDR bd_addr;
            bd_addr.nap = strtol(argv[2], NULL, 0);
            bd_addr.uap = strtol(argv[3], NULL, 0);
            bd_addr.lap = strtol(argv[4], NULL, 0);
            LOG_I("conn addr: %04X:%02X:%06lX",
                  bd_addr.nap,
                  bd_addr.uap,
                  bd_addr.lap);
            bt_cm_connect_req(&bd_addr, atoi(argv[5]));
        }
        else if (strcmp(argv[1], "delete") == 0)
        {
            bt_cm_delete_bonded_devs();
        }
        else if (strcmp(argv[1], "close") == 0)
        {
            bt_close_bt_request();
        }
        else if (strcmp(argv[1], "open") == 0)
        {

            if (argc == 2)
                bt_open_bt_request();
            else if (argc == 3)
            {
                if (strcmp(argv[2], "inquiry") == 0)
                {
                    bt_open_bt_request_scan(1);
                }
                else if (strcmp(argv[2], "page") == 0)
                {
                    bt_open_bt_request_scan(2);
                }
            }
        }
        else if (strcmp(argv[1], "rd_addr") == 0)
        {
            gap_rd_local_bd_req(bts2_task_get_app_task_id());
        }
        else if (strcmp(argv[1], "scan") == 0)
        {
            uint8_t is_on = atoi(argv[2]);
            gap_wr_scan_enb_req(bts2_task_get_app_task_id(), is_on, is_on);
        }
        else if (strcmp(argv[1], "ch_addr") == 0)
        {
            uint8_t addr_type = atoi(argv[2]);
            uint8_t addr_method = atoi(argv[3]);
            BTS2S_BD_ADDR bd_addr;
            bd_addr_t bd_addr_c;
            bd_addr.nap = strtol(argv[4], NULL, 0);
            bd_addr.uap = strtol(argv[5], NULL, 0);
            bd_addr.lap = strtol(argv[6], NULL, 0);
            LOG_I("conn addr: %04X:%02X:%06lX",
                  bd_addr.nap,
                  bd_addr.uap,
                  bd_addr.lap);

            bt_addr_convert_to_general(&bd_addr, &bd_addr_c);
            sibles_change_bd_addr(addr_type, addr_method, &bd_addr_c);
        }
        else if (strcmp(argv[1], "en_btlog") == 0)
        {
            uint8_t type = atoi(argv[2]);
            if (type == 0) // FSM
            {
                uint8_t is_on = atoi(argv[3]);
                if (is_on)
                    bt_fsm_hook_set(bt_fsm_hook_fun);
                else
                    bt_fsm_hook_set(NULL);
            }
        }
        else if (strcmp(argv[1], "dut") == 0)
        {
            gap_enb_dut_mode_req(bts2_task_get_app_task_id());
        }
        else if (strcmp(argv[1], "sleep") == 0)
        {
#ifdef SOC_SF32LB52X
            uint8_t is_enable = atoi(argv[2]);
            if (is_enable == 0)
                bt_sleep_control(0);
            else
                bt_sleep_control(1);
#else
            LOG_E("Not Support!!!");
#endif
        }
        else if (strcmp(argv[1], "uart_dut") == 0)
        {
            gap_enb_dut_mode_req(bts2_task_get_app_task_id());
            rt_thread_mdelay(500);
#ifndef BSP_USING_PC_SIMULATOR
#ifdef BT_RF_TEST
            {
                extern void uart_ipc_path_change(void);
                uart_ipc_path_change();
            }
#endif //BT_RF_TEST
#endif
        }
        else if (strcmp(argv[1], "get_gap_mode") == 0)
        {
            uint8_t mod_str[3][7] = {"Active", "Hold", "Sniff"};
            bt_cm_device_manager_t *env = bt_cm_get_env();
            for (uint8_t i = 0; i < BT_CM_MAX_BOND; i++)
            {
                bt_cm_dev_acl_info_t *conn = &env->bt_devices[i];
                if (conn->state >= BT_CM_ACL_STATE_CONNECTED)
                {
                    if (conn->link_mode > PARK_MODE)
                    {
                        LOG_W("abnormal mode %d", conn->link_mode);
                    }
                    else
                    {
                        LOG_D("%s mode st: %d, inv: %.2f", mod_str[conn->link_mode], conn->link_mode, conn->link_interval);
                    }
                }
            }
        }
        else if (strcmp(argv[1], "exit_sniff") == 0)
        {
            bt_cm_device_manager_t *env = bt_cm_get_env();
            uint8_t i;
            for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
            {
                if (env->bt_devices[i].state >= BT_CM_ACL_STATE_CONNECTED)
                    hcia_exit_sniff_mode(&env->bt_devices[i].info.bd_addr, NULL);
            }
        }
        else if (strcmp(argv[1], "get_link_key") == 0)
        {
            uint8_t i;
            for (i = 0; i < BT_CM_MAX_BOND; i++)
            {
                if (g_bt_bonded_dev.info[i].is_use)
                {
                    sc_rd_paired_dev_link_key_req(bts2_task_get_app_task_id(), &g_bt_bonded_dev.info[i].bd_addr);
                }
            }
        }
        else if (strcmp(argv[1], "get_name") == 0)
        {
            bt_cm_device_manager_t *env = bt_cm_get_env();
            uint8_t i;
            for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
            {
                if (env->bt_devices[i].state >= BT_CM_ACL_STATE_CONNECTED)
                    gap_rd_rmt_name_req(bts2_task_get_app_task_id(), env->bt_devices[i].info.bd_addr);
            }
        }
#ifdef BSP_BQB_TEST
        else if (strcmp(argv[1], "av_conn") == 0)
        {
            BTS2S_BD_ADDR bd_addr;
            bd_addr.nap = strtol(argv[2], NULL, 0);
            bd_addr.uap = strtol(argv[3], NULL, 0);
            bd_addr.lap = strtol(argv[4], NULL, 0);
            LOG_I("conn addr: %04X:%02X:%06lX",
                  bd_addr.nap,
                  bd_addr.uap,
                  bd_addr.lap);
            bt_av_conn(&bd_addr, AV_SNK);
        }
        else if (strcmp(argv[1], "av_disconn") == 0)
        {
            bt_av_disconnect(0);
        }
        else if (strcmp(argv[1], "avrcp_conn") == 0)
        {
            BTS2S_BD_ADDR bd_addr;
            bd_addr.nap = strtol(argv[2], NULL, 0);
            bd_addr.uap = strtol(argv[3], NULL, 0);
            bd_addr.lap = strtol(argv[4], NULL, 0);
            LOG_I("conn addr: %04X:%02X:%06lX",
                  bd_addr.nap,
                  bd_addr.uap,
                  bd_addr.lap);
            if (atoi(argv[5]))
                avrcp_conn_req(bts2_task_get_app_task_id(), bd_addr, AVRCP_TG, AVRCP_CT);
            else
                avrcp_conn_req(bts2_task_get_app_task_id(), bd_addr, AVRCP_CT, AVRCP_TG);
        }
        else if (strcmp(argv[1], "release_a2dp") == 0)
        {
            bt_av_release_stream(0);
        }
#endif
#ifdef CFG_SPP_LOOPBACK
        else if (strcmp(argv[1], "spp_loopback") == 0)
        {
            LOG_I("set loopback: %d", atoi(argv[2]));
            bt_spp_set_spp_data_loopback_enable(atoi(argv[2]));
        }
#endif
        else if (strcmp(argv[1], "rd_ext") == 0)
        {
            bt_cm_device_manager_t *env = bt_cm_get_env();
            uint32_t i;
            for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
            {
                if (env->bt_devices[i].state >= BT_CM_ACL_STATE_CONNECTED)
                    gap_rd_rmt_ext_featr_req(bts2_task_get_app_task_id(), atoi(argv[2]), env->bt_devices[i].info.bd_addr);
            }
        }
    }
}

#ifdef RT_USING_FINSH
    MSH_CMD_EXPORT(bt_cm, BT connection manager command);
#endif // RT_USING_FINSH


#endif // BSP_BT_CONNECTION_MANAGER
