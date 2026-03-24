/*
 * SPDX-FileCopyrightText: 2026-2026 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BF0_BLE_SVC_CHANGE_H_
#define _BF0_BLE_SVC_CHANGE_H_

#ifdef BLE_SVC_CHG_ENABLE

#ifdef BSP_BLE_CONNECTION_MANAGER
    #include "ble_connection_manager.h"
    #define SVC_CHANGE_MAX_CONN_NUM MAX_CONNECTION_LINK_NUM
#else
    #define SVC_CHANGE_MAX_CONN_NUM 8
#endif

typedef struct
{
    uint8_t need_send_ind;
    uint8_t state;
    uint8_t remote_index;
    uint16_t remote_handle;
    uint16_t cccd_hdl;
    uint16_t svc_start_handle;
    uint16_t svc_end_handle;

    uint8_t svc_changed_ccc[SVC_CHANGE_MAX_CONN_NUM];
} ble_svc_chang_env_t;

typedef enum
{
    SVC_CHANGE_STATE_IDLE,
    SVC_CHANGE_STATE_SEARCHING,
    SVC_CHANGE_STATE_SEARCH_COMPLETED,
    SVC_CHANGE_STATE_CONFIGURAING,
    SVC_CHANGE_STATE_READY,
} ble_svc_change_state_t;

typedef enum
{
    SVC_CHANGE_IND_SEND_TYPE_NEVER,
    SVC_CHANGE_IND_SEND_TYPE_ONCE,
    SVC_CHANGE_IND_SEND_TYPE_ALWAYS,
} ble_svc_change_ind_send_type_t;

void ble_svc_changed_send_set(uint8_t send_type);

#endif // BLE_SVC_CHG_ENABLE

#endif // _BF0_BLE_SVC_CHANGE_H_
