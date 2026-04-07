/*
 * SPDX-FileCopyrightText: 2026-2026 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "bf0_sibles.h"
#include "bf0_ble_err.h"
#include "att.h"

#include "bf0_ble_gap_internal.h"
#include "bf0_sibles_svc_change.h"

#define LOG_TAG "svc_change"
#include "log.h"

#ifdef BLE_SVC_CHG_ENABLE

ble_svc_chang_env_t g_svc_changed_env = {.need_send_ind = 0, .state = SVC_CHANGE_STATE_IDLE};

ble_svc_chang_env_t *svc_get_env()
{
    return &g_svc_changed_env;
}

const uint8_t generic_attribute_uuid[ATT_UUID_16_LEN] =
{
    0x01, 0x18,
};

const uint8_t svc_changed_uuid[ATT_UUID_16_LEN] =
{
    0x05, 0x2a,
};

uint8_t ble_svc_change_enable(uint8_t conn_idx)
{
    ble_svc_chang_env_t *env = svc_get_env();

    if (env->state >= SVC_CHANGE_STATE_SEARCHING)
    {
        LOG_I("svc change fail to enable %d", env->state);
        return 0xFF;
    }

    LOG_I("ble_svc_change_enable");
    env->remote_index = conn_idx;
    env->state = SVC_CHANGE_STATE_SEARCHING;
    sibles_search_service(conn_idx, ATT_UUID_16_LEN, (uint8_t *)generic_attribute_uuid);
    return 0;
}

void ble_svc_changed_send_set(uint8_t send_type)
{
    LOG_I("ble_svc_changed_send_set %d", send_type);
    ble_svc_chang_env_t *env = svc_get_env();
    env->need_send_ind = send_type;
}

static void send_svc_changed_indication(uint8_t conn_idx)
{
    ble_svc_chang_env_t *env = svc_get_env();

    if (env->need_send_ind == SVC_CHANGE_IND_SEND_TYPE_NEVER)
    {
        return;
    }

    if (env->svc_changed_ccc[conn_idx] == INDICATION_ENABLE_VALUE
#ifdef BSP_BLE_CONNECTION_MANAGER
            && connection_manager_get_enc_state(conn_idx) == ENC_STATE_ON
#endif
       )
    {
        LOG_I("send svc changed ind %d", conn_idx);
        sibles_send_svc_changed_ind(conn_idx, 1, 0xFFFF);

        if (env->need_send_ind == SVC_CHANGE_IND_SEND_TYPE_ONCE)
        {
            env->need_send_ind = SVC_CHANGE_IND_SEND_TYPE_NEVER;
        }
    }
}


int8_t ble_svc_changed_cccd()
{
    ble_svc_chang_env_t *env = svc_get_env();

    sibles_write_remote_value_t value;
    uint16_t enable = 2;
    value.handle = env->cccd_hdl;
    value.write_type = SIBLES_WRITE;
    value.len = 2;
    value.value = (uint8_t *)&enable;

    int8_t retry_index = 0;
    int8_t max_retry_time = 5;

    int8_t ret = SIBLES_WRITE_NO_ERR;

    while (retry_index < max_retry_time)
    {
        ret = sibles_write_remote_value(env->remote_handle, env->remote_index, &value);

        if (ret == SIBLES_WIRTE_TX_FLOWCTRL_ERR)
        {
            retry_index++;
        }
        else
        {
            break;
        }
    }

    return ret;
}

static int ble_svc_change_event_handler(uint16_t event_id, uint8_t *data, uint16_t len)
{
    ble_svc_chang_env_t *env = svc_get_env();

    switch (event_id)
    {
    case SIBLES_REGISTER_REMOTE_SVC_RSP:
    {
        sibles_register_remote_svc_rsp_t *rsp = (sibles_register_remote_svc_rsp_t *)data;

        if (rsp->status != HL_ERR_NO_ERROR)
        {
            LOG_I("svc changed register failed with %d", rsp->status);
            env->state = SVC_CHANGE_STATE_IDLE;
            break;
        }

        env->state = SVC_CHANGE_STATE_SEARCH_COMPLETED;
        int configure_result = ble_svc_changed_cccd();
        if (configure_result == SIBLES_WRITE_NO_ERR)
        {
            LOG_I("svc changed register success");
            env->state = SVC_CHANGE_STATE_READY;
        }
        else
        {
            LOG_I("svc changed write cccd failed with %d", configure_result);
            env->state = SVC_CHANGE_STATE_IDLE;
        }
        break;
    }
    case SIBLES_REMOTE_EVENT_IND:
    {
        if (env->state != SVC_CHANGE_STATE_READY)
        {
            LOG_I("svc change state error %d", env->state);
            return 0;
        }

        // remote service have changed
        uint16_t start_handle, end_handle;

        sibles_remote_event_ind_t *ind = (sibles_remote_event_ind_t *)data;
        memcpy(&start_handle, ind->value, 2);
        memcpy(&end_handle, ind->value + 2, 2);

        sibles_send_remote_svc_change_ind(ind->conn_idx, start_handle, end_handle);
        break;
    }
    default:
        break;
    }
    return 0;
}

int service_changed_ble_handler(uint16_t event_id, uint8_t *data, uint16_t len, uint32_t context)
{
    ble_svc_chang_env_t *env = svc_get_env();
    switch (event_id)
    {
    case BLE_GAP_CONNECTED_IND:
    {
        ble_gap_connect_ind_t *ind = (ble_gap_connect_ind_t *)data;
        env->svc_changed_ccc[ind->conn_idx] = 0;
        break;
    }
    case BLE_GAP_DISCONNECTED_IND:
    {
        ble_gap_disconnected_ind_t *ind = (ble_gap_disconnected_ind_t *)data;
        if (env->remote_index == ind->conn_idx)
        {
            LOG_I("svc changed ind unregister");
            env->state = SVC_CHANGE_STATE_IDLE;
            sibles_unregister_remote_svc(env->remote_index, env->svc_start_handle, env->svc_end_handle, ble_svc_change_event_handler);
        }
        break;
    }
    case SIBLES_SVC_CHANGED_CFG:
    {
        // remote register svc changed ind
        sibles_svc_changed_cfg_t *rsp = (sibles_svc_changed_cfg_t *)data;
        env->svc_changed_ccc[rsp->conn_idx] = rsp->ind_cfg;

        send_svc_changed_indication(rsp->conn_idx);
        break;
    }
    case BLE_GAP_BOND_IND:
    {
        ble_gap_bond_ind_t *ind = (ble_gap_bond_ind_t *)data;
        switch (ind->info)
        {
        case GAPC_LTK_EXCH:
        {
            ble_svc_change_enable(ind->conn_idx);
            break;
        }
        default:
            break;
        }
        break;
    }
    case BLE_GAP_ENCRYPT_IND:
    {
        ble_gap_encrypt_ind_t *ind = (ble_gap_encrypt_ind_t *)data;

        send_svc_changed_indication(ind->conn_idx);
        ble_svc_change_enable(ind->conn_idx);
        break;
    }
    case SIBLES_SEARCH_SVC_RSP:
    {
        sibles_svc_search_rsp_t *rsp = (sibles_svc_search_rsp_t *)data;

        if (env->remote_index != rsp->conn_idx)
        {
            break;
        }

        if (env->state != SVC_CHANGE_STATE_SEARCHING)
        {
            break;
        }

        if (rsp->result != HL_ERR_NO_ERROR)
        {
            env->state = SVC_CHANGE_STATE_IDLE;
            break;
        }

        if (memcmp(rsp->search_uuid, generic_attribute_uuid, rsp->search_svc_len) == 0)
        {
            LOG_I("generic attribute");

            sibles_svc_search_char_t *chara = (sibles_svc_search_char_t *)rsp->svc->att_db;
            uint8_t find = 0;
            for (int i = 0; i < rsp->svc->char_count; i++)
            {
                if (memcmp(chara->uuid, svc_changed_uuid, chara->uuid_len) == 0)
                {
                    LOG_I("noti_uuid received, att handle(%x), des handle(%x)", chara->attr_hdl, chara->desc[0].attr_hdl);
                    env->cccd_hdl = sibles_descriptor_handle_find(chara, ATT_DESC_CLIENT_CHAR_CFG);
                    find = 1;
                    break;
                }
            }

            if (find == 0)
            {
                env->state = SVC_CHANGE_STATE_IDLE;
                break;
            }

            env->svc_start_handle = rsp->svc->hdl_start;
            env->svc_end_handle = rsp->svc->hdl_end;

            env->remote_index = rsp->conn_idx;
            env->remote_handle = sibles_register_remote_svc(rsp->conn_idx, rsp->svc->hdl_start, rsp->svc->hdl_end, ble_svc_change_event_handler);
            LOG_I("svc change register handle %d", env->remote_handle);
        }
        break;
    }

    default:
        break;
    }
    return 0;
}

BLE_EVENT_REGISTER(service_changed_ble_handler, NULL);
#endif //BLE_SVC_CHG_ENABLE