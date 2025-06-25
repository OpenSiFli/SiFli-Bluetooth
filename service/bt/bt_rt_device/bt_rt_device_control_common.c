/*
 * SPDX-FileCopyrightText: 2022-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>
#include <string.h>
#include "drv_bt.h"
#include "bt_rt_device.h"
#include "bts2_global.h"
#include "bts2_app_inc.h"
#include "bf0_ble_common.h"
#include "bf0_sibles.h"
#include "bt_connection_manager.h"
#ifdef BT_USING_AVRCP
    #include "bt_rt_device_control_avrcp.h"
#endif

#ifdef AUDIO_USING_MANAGER
    #include "audio_server.h"
#endif // AUDIO_USING_MANAGER

#define DBG_TAG               "bt_rt_device.control_comm"
//#define DBG_LVL               DBG_INFO
#include <log.h>


static bd_addr_t g_custom_bt_addr = {0};

uint8_t bt_sifli_get_role_profile_connect_status(rt_bt_device_t *dev, bt_cm_link_type_t link_type, bt_profile_t profile)
{
#if defined(BT_CONNECT_SUPPORT_MULTI_LINK) && (BT_CM_DEVICE_MAX_CONN > 1)
    uint32_t i;
    for (i = 0; i < BT_CM_DEVICE_MAX_CONN; i++)
    {
        if (rt_bt_get_connect_dev_by_idx(dev, i)->link_type == link_type &&
                BT_STATE_CONNECTED == rt_bt_get_connect_state_by_conn_idx(dev, i, profile))
        {
            return 1;
        }
    }
#else
    if (BT_STATE_CONNECTED == rt_bt_get_connect_state(dev, profile))
    {
        return 1;
    }
#endif
    return 0;
}
extern void set_speaker_volume(uint8_t volume);
static bt_err_t bt_sifli_set_speaker_volume(rt_bt_device_t *dev, bt_volume_set_t *set)
{
    bt_err_t ret = BT_ERROR_STATE;
    int volume;
    if (set->volume.call_volume > 15)
    {
        volume = 15;
    }
    else
    {
        volume =  set->volume.call_volume;
    }
    if (!bt_sifli_get_role_profile_connect_status(dev, BT_LINK_PHONE, BT_PROFILE_HFP))
        return ret;
    if (!bt_sifli_check_bt_event(BT_SET_VGS_EVENT))
    {
        bt_sifli_set_bt_event(BT_SET_VGS_EVENT);
        ret = bt_interface_set_speaker_volume(volume);
    }

#ifdef AUDIO_USING_MANAGER
    audio_server_set_private_volume(AUDIO_TYPE_BT_VOICE, volume);
#endif // AUDIO_USING_MANAGER
    return ret;
}

static bt_err_t bt_sifli_control_close(int cmd)
{
    bt_err_t ret = BT_EOK;
    //gap_wr_scan_enb_req(bts2_task_get_app_task_id(), FALSE, FALSE);
    bt_sifli_set_bt_event(BT_SET_CLOSE_EVENT);
    if (BT_CONTROL_DISCONNECT == cmd)
    {
        bt_sifli_set_bt_event(BT_SET_DIS_GAP_EVENT);
        ret = BT_ERROR_IN_PROGRESS;
    }

    if (BT_CONTROL_CANCEL_PAGE == cmd)
    {
        bt_sifli_set_bt_event(BT_SET_CANCEL_PAGE_EVENT);
        ret = BT_ERROR_IN_PROGRESS;
    }

    bt_interface_close_bt();
    //shutdown stop waiting paring complete to avoid timeout erro
    //bt_sifli_wait_pairing_complete();
    LOG_I("BT_CONTROL_CLOSE_DEVICE cmd:%x", cmd);
    return ret;
}

static bt_err_t bt_sifli_control_open(void)
{
    bt_err_t ret = BT_EOK;

    bt_sifli_set_bt_event(BT_SET_OPEN_EVENT);
    if (bt_sifli_check_bt_event(BT_SET_CLOSE_EVENT))
    {
        LOG_I("BT is not closed and needs to wait");
        ret = BT_ERROR_STATE;
    }
    else
    {
        bt_interface_open_bt();
        bt_sifli_reset_bt_event(BT_SET_OPEN_EVENT);
        LOG_I("BT_CONTROL_OPEN_DEVICE");
    }
    return ret;
}

static uint8_t bt_sifli_get_role_connect(struct rt_bt_device *bt_handle, bt_cm_link_type_t link_type)
{
    uint8_t i;
    for (i = 0; i < BT_MAX_ACL_NUM; i ++)
    {
        if (BT_STATE_ACL_CONNECTED == rt_bt_get_acl_state_by_conn_idx(bt_handle, i) &&
                rt_bt_get_connect_dev_by_idx(bt_handle, i)->link_type == link_type)
        {
            return 1;
        }
    }
    return 0;
}
bt_err_t bt_sifli_control_common(struct rt_bt_device *bt_handle, int cmd, void *args)
{
    bt_err_t ret = BT_EOK;

    switch (cmd)
    {
    case BT_CONTROL_CLOSE_DEVICE:
    {
        bt_cm_disconnect_req();
        bt_sifli_reset_bt_event(BT_SET_OPEN_EVENT | BT_SET_DIS_GAP_EVENT | BT_SET_CANCEL_PAGE_EVENT);
        ret = bt_sifli_control_close(cmd);
    }
    break;

    case BT_CONTROL_OPEN_DEVICE:
    {
        ret = bt_sifli_control_open();
    }
    break;

    case BT_CONTROL_SET_SCAN:
    {
        if (app_bt_get_non_signaling_test_status())
            return BT_ERROR_STATE;
        bt_scan_con_t *scan = (bt_scan_con_t *)args;
        bt_interface_set_scan_mode(scan->inquiry_scan, scan->page_scan);
    }
    break;

    case BT_CONTROL_GET_SCAN:
    {
        uint8_t *p_args = (uint8_t *)args;
        *p_args = bt_interface_get_current_scan_mode();
    }
    break;

    case BT_CONTROL_EXIT_SNIFF:
    {
        bt_interface_exit_sniff_mode((unsigned char *)args);
    }
    break;
    case BT_CONTROL_SET_LINK_POLICY:
    {
        bt_set_link_policy_t *info = (bt_set_link_policy_t *)args;
        bt_interface_wr_link_policy_setting((unsigned char *)&info->mac, info->mode);
    }
    break;

    case BT_CONTROL_DEVICE_INIT:
    {
    }
    break;

    case BT_CONTROL_DEVICE_DEINIT:
    {
        /*TODO*/
    }
    break;

    case BT_CONTROL_SWITCH_TO_SOURCE:
    case BT_CONTROL_SWITCH_TO_SINK:

        break;

    case BT_CONTROL_SEARCH_EQUIPMENT:
    {
        bt_sifli_set_bt_event(BT_SET_INQ_EVENT);
        bt_interface_start_inquiry();
        LOG_D("BT_CONTROL_SEARCH_EQUIPMENT\n");
    }
    break;

    case BT_CONTROL_CANCEL_SEARCH:
    {
        bt_sifli_reset_bt_event(BT_SET_INQ_EVENT);
        bt_interface_stop_inquiry();
        LOG_D("BT_CONTROL_CANCEL_SEARCH\n");
    }
    break;

    case BT_CONTROL_CONNECT_DEVICE:
    {
        bt_mac_t *mac = (bt_mac_t *)args;

        if (BT_ROLE_MASTER == bt_handle->role)
        {
            /*@TODO*/
        }
        else
        {
            ret = bt_interface_hfp_hf_start_connecting((unsigned char *)(mac->addr));
        }
    }
    break;

    case BT_CONTROL_CONNECT_DEVICE_EX:
    {
        ret = BT_EOK;
        bt_connect_info_t *info = (bt_connect_info_t *)args;
        bt_mac_t *mac = &(info->mac);
        bt_interface_set_scan_mode(0, 0);
        bt_cm_dev_info_t *conn = bt_cm_get_bonded_dev_by_addr((uint8_t *)&info->mac);
        LOG_D("[%s] conn:%p type:%d", __func__, conn, conn->link_type);
        if (conn)
        {
            bt_connect_dev_t conn_dev;
            conn_dev.link_type = conn->link_type;
            conn_dev.connect = 1;
            memcpy(&conn_dev.mac, &info->mac, sizeof(bt_mac_t));
            rt_bt_add_connect_dev(bt_handle, &conn_dev);
            if (BT_LINK_EARPHONE == conn->link_type)
                ret =  bt_interface_conn_to_source_ext((unsigned char *)(mac->addr), info->profile);
            else
                ret = bt_interface_conn_ext((unsigned char *)(mac->addr), info->profile);
        }
        else
        {
            bt_connect_dev_t conn_dev;
            memcpy(&conn_dev.mac, &info->mac, sizeof(bt_mac_t));
            conn_dev.connect = 1;
            if (BT_ROLE_MASTER == bt_handle->role && !bt_sifli_get_role_connect(bt_handle, BT_LINK_EARPHONE))
            {
                conn_dev.link_type = BT_LINK_EARPHONE;
                rt_bt_add_connect_dev(bt_handle, &conn_dev);
                ret =  bt_interface_conn_to_source_ext((unsigned char *)(mac->addr), info->profile);
            }
            else if (BT_ROLE_SLAVE == bt_handle->role && !bt_sifli_get_role_connect(bt_handle, BT_LINK_PHONE))
            {
                conn_dev.link_type = BT_LINK_PHONE;
                rt_bt_add_connect_dev(bt_handle, &conn_dev);
                ret = bt_interface_conn_ext((unsigned char *)(mac->addr), info->profile);
            }
            else
                ret = BT_ERROR_STATE;
        }
        info->conn_idx = rt_bt_get_connect_idx_by_mac(bt_handle, &info->mac);
    }
    break;

    case BT_CONTROL_DISCONNECT:
    {
        ret = bt_sifli_control_close(cmd);
    }
    break;

    case BT_CONTROL_DISCONNECT_EX:
    {
        bt_profile_t *profile = (bt_profile_t *)args;
        ret = bt_interface_disc_ext((unsigned char *)&rt_bt_get_connect_dev_by_idx(bt_handle, 0)->mac, *profile);
    }
    break;
    case BT_CONTROL_DISCONNECT_PROFILE:
    {
        bt_profile_info_t *info = (bt_profile_info_t *)args;
        bt_connect_dev_t *dev_conn = rt_bt_get_connect_dev_by_idx(bt_handle, info->conn_idx);
        if (dev_conn == NULL)
        {
            ret = BT_ERROR_INPARAM;
            break;
        }
        ret = bt_interface_disc_ext((unsigned char *)&dev_conn->mac, info->profile);
    }
    break;

    case BT_CONTROL_QUERY_STATE_EX:

        break;

    case BT_CONTROL_QUERY_STATE_NONBLOCK:
    {
        //ret = fr508x_query_state_nonblock();
    }
    break;

    case BT_CONTROL_AUDIO_TRANSFER_EX:
    {
        bt_hfp_audio_switch_t *set = (bt_hfp_audio_switch_t *)args;
        bt_cm_dev_info_t *conn = bt_cm_get_bonded_dev_by_addr((uint8_t *)&set->peer_addr);
        LOG_I("[%s] conn:%p link_type:%d type:%d", __func__, conn, conn->link_type, set->type);
#ifdef BT_USING_AG
        if (conn && conn->link_type == BT_LINK_EARPHONE)
        {
            BTS2S_BD_ADDR dest_addr;
            bt_addr_convert_to_bts((bd_addr_t *)set->peer_addr.addr, &dest_addr);
            if (set->type)
                bt_hfp_disconnect_audio(&dest_addr);
            else
                bt_hfp_connect_audio(&dest_addr);
        }
#endif
#ifdef BT_USING_HF
        if (conn && conn->link_type == BT_LINK_PHONE)
        {
            bt_interface_audio_switch(set->type);
        }
#endif
    }
    break;

    case BT_CONTROL_SET_VOLUME:
    {
        bt_volume_set_t *set = (bt_volume_set_t *)args;
        if (BT_VOLUME_CALL == set->mode)
        {
            ret = bt_sifli_set_speaker_volume(bt_handle, set);
        }
        else if (BT_VOLUME_MEDIA == set->mode)
        {
#ifdef BT_USING_AVRCP
            ret = bt_sifli_set_avrcp_volume(bt_handle, set);
#else
            ret = BT_ERROR_STATE;
#endif
        }
    }
    break;

    case BT_CONTROL_SET_LOCAL_NAME:
    {
        set_name_t   *p_args;
        p_args = (set_name_t *)args;
        bt_interface_set_local_name(p_args->size, (void *)(p_args->name));
    }
    break;

    case BT_CONTROL_RD_LOCAL_NAME:
    {
        bt_sifli_set_bt_event(BT_SET_RD_LOCAL_NAME_EVENT);
        bt_interface_rd_local_name();
    }
    break;

    case BT_CONTROL_RD_LOCAL_RSSI:
    {
        bt_mac_t *mac = (bt_mac_t *)args;
        bt_sifli_set_bt_event(BT_SET_RD_LOCAL_RSSI_EVENT);
        bt_interface_rd_local_rssi((unsigned char *)(mac->addr));
    }
    break;

    case BT_CONTROL_GET_BT_MAC:
    {
        bt_mac_t *p_args = (bt_mac_t *)args;
        bd_addr_t addr = {0};
        if (rt_memcmp(&g_custom_bt_addr, &addr, sizeof(bd_addr_t)))
        {
            rt_memcpy(p_args->addr, g_custom_bt_addr.addr, BT_MAX_MAC_LEN);
            return ret;
        }
        ret = ble_get_public_address(&addr);
        if (ret == 0)
        {
            rt_memcpy(p_args->addr, addr.addr, BT_MAX_MAC_LEN);
        }
        else
        {
            ret = BT_ERROR_RESP_FAIL;
        }
    }
    break;
#if defined(BT_USING_MIC_MUTE) && defined(AUDIO_USING_MANAGER)
    case BT_CONTROL_SET_MIC_MUTE:
    {
        bt_mic_mute_t *p_args = (bt_mic_mute_t *)args;
        audio_server_set_public_mic_mute(*p_args);
    }
    break;
    case BT_CONTROL_GET_MIC_MUTE:
    {
        bt_mic_mute_t *p_args = (bt_mic_mute_t *)args;
        *p_args = audio_server_get_public_mic_mute();
    }
    break;
#endif

    case BT_CONTROL_CHANGE_BD_ADDR:
    {
        if (RT_NULL == args)
        {
            bd_addr_t addr = {0};
            ble_get_public_address(&addr);
            sibles_change_bd_addr(SIBLES_CH_BD_TYPE_BT, SIBLES_CH_BD_METHOD_CUSTOMIZE, &addr);
#ifdef BT_FINSH_PAN
            BTS2S_BD_ADDR     bd_addr;
            bt_addr_convert_to_bts(&addr, &bd_addr);
            bt_interface_update_pan_addr(&bd_addr);
#endif
            LOG_I("change bd_addr:%02X:%02X:%02X:%02X:%02X:%02X", addr.addr[0], addr.addr[1], addr.addr[2],
                  addr.addr[3], addr.addr[4], addr.addr[5]);
        }
        else
        {
            rt_memcpy(&g_custom_bt_addr, args, sizeof(bd_addr_t));
            sibles_change_bd_addr(SIBLES_CH_BD_TYPE_BT, SIBLES_CH_BD_METHOD_CUSTOMIZE, args);
#ifdef BT_FINSH_PAN
            BTS2S_BD_ADDR     bd_addr;
            bt_addr_convert_to_bts(&g_custom_bt_addr, &bd_addr);
            bt_interface_update_pan_addr(&bd_addr);
#endif
            LOG_I("change bd_addr:%02X:%02X:%02X:%02X:%02X:%02X", g_custom_bt_addr.addr[0], g_custom_bt_addr.addr[1], g_custom_bt_addr.addr[2],
                  g_custom_bt_addr.addr[3], g_custom_bt_addr.addr[4], g_custom_bt_addr.addr[5]);
        }
    }
    break;

    case BT_CONTROL_CANCEL_PAGE:
    {
        ret = bt_sifli_control_close(cmd);
    }
    break;

    case BT_CONTROL_CANCEL_PAGE_BY_ADDR:
    {
        bt_interface_cancel_connect_req((unsigned char *)args);
    }
    break;

    case BT_CONTROL_GET_RMT_VERSION:
    {
        BTS2S_BD_ADDR bd_addr;
        bt_mac_t *mac_addr = (bt_mac_t *)args;
        bt_addr_convert_to_bts((bd_addr_t *)mac_addr, &bd_addr);
        gap_rd_rmt_version_req(bts2_task_get_app_task_id(), bd_addr);
    }
    break;

#ifdef BT_USING_PAIRING_CONFIRMATION
    case BT_CONTROL_IO_CAPABILITY_RES:
    {
        bt_io_capability_rsp_t *res = (bt_io_capability_rsp_t *)args;
        BTS2S_BD_ADDR bd_addr;
        bt_addr_convert_to_bts((bd_addr_t *)&res->mac, &bd_addr);
        sc_io_capability_rsp(&bd_addr, (BTS2E_SC_IO_CAPABILITY)res->io_capability, res->mitm, res->bonding);
    }
    break;

    case BT_CONTROL_USER_CONFIRM_RES:
    {
        bt_user_confirm_rsp_t *res = (bt_user_confirm_rsp_t *)args;
        BTS2S_BD_ADDR bd_addr;
        bt_addr_convert_to_bts((bd_addr_t *)&res->mac, &bd_addr);
        sc_user_cfm_rsp(&bd_addr, res->confirm);
        if (!res->confirm)
        {
            ret = bt_sifli_control_close(BT_CONTROL_DISCONNECT);
        }
    }
    break;
#endif

    case BT_CONTROL_SWITCH_ON:
    {
        if (app_bt_get_non_signaling_test_status())
            return BT_ERROR_STATE;
        bt_interface_set_scan_mode(TRUE, TRUE);
    }
    break;

    case BT_CONTROL_SWITCH_OFF:
    {
        if (app_bt_get_non_signaling_test_status())
            return BT_ERROR_STATE;
        bt_interface_set_scan_mode(FALSE, FALSE);
    }
    break;

    case BT_CONTROL_DISCONNECT_BY_CONNIDX:
    {
        BTS2S_BD_ADDR addr = {0};
        uint8_t *conn_idx = (uint8_t *)args;
        bt_connect_dev_t *dev_conn = rt_bt_get_connect_dev_by_idx(bt_handle, *conn_idx);
        bt_addr_convert_to_bts((bd_addr_t *)&dev_conn->mac, &addr);
        LOG_I("[%s] conn_idx:%d, bd addr:%04x-%02x-%06x", __func__, *conn_idx, addr.nap, addr.uap, addr.lap);
        gap_disconnect_req(&addr);
    }
    break;

    case BT_CONTROL_GET_RMT_NAME:
    {
        bt_mac_t *mac = (bt_mac_t *)args;
        BTS2S_BD_ADDR addr;
        bt_addr_convert_to_bts((bd_addr_t *)mac, &addr);
        gap_rd_rmt_name_req(bts2_task_get_app_task_id(), addr);
    }
    break;

    case BT_CONTROL_ACPT_CONNECT:
    {
        bt_acpt_connect_t *info = (bt_acpt_connect_t *)args;
        bt_connect_dev_t dev_info;
        memcpy(&dev_info.mac, &info->addr, sizeof(bt_mac_t));
        dev_info.link_type = info->link_type;
        dev_info.connect = 1;
        rt_bt_add_connect_dev(bt_handle, &dev_info);
        ret = bt_interface_acpt_connect_req((unsigned char *)&info->addr, info->link_role, info->link_type);
    }
    break;
    case BT_CONTROL_REJECT_CONNECT:
    {
        bt_reject_connect_t *info = (bt_reject_connect_t *)args;
        ret = bt_interface_reject_connect_req((unsigned char *)&info->addr, info->reason);
    }
    break;
    default:
        ret = BT_ERROR_UNSUPPORTED;
        break;
    }
    return ret;
}









