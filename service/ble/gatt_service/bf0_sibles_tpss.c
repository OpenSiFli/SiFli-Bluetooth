/*
 * SPDX-FileCopyrightText: 2025-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "bf0_sibles.h"
#include "bf0_ble_tpss.h"

#ifdef BSP_BLE_TPSS

#define LOG_TAG "ble_tx_power"
#include "log.h"


/// TX POWER Service Attributes Indexes
enum
{
    TX_POWER_SERVICE_IDX_SVC,

    TX_POWER_SERVICE_IDX_POWER_LVL_CHAR,
    TX_POWER_SERVICE_IDX_POWER_LVL_VAL,
    TX_POWER_SERVICE_IDX_POWER_LVL_NTF_CFG,

    TX_POWER_SERVICE_IDX_NB,
};

typedef enum
{
    TX_POWER_STATE_IDLE,
    TX_POWER_STATE_READY,
    TX_POWER_STATE_BUSY,
} ble_tx_power_state_t;



const struct attm_desc tx_power_att_db[TX_POWER_SERVICE_IDX_NB] =
{
    // TX POWER Service Declaration
    [TX_POWER_SERVICE_IDX_SVC]                   =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},

    // TX POWER Level Characteristic Declaration
    [TX_POWER_SERVICE_IDX_POWER_LVL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
    // TX POWER Level Characteristic Value
    [TX_POWER_SERVICE_IDX_POWER_LVL_VAL]         =   {ATT_CHAR_TX_POWER_LEVEL,    PERM(RD, ENABLE) | PERM(NTF, ENABLE), PERM(RI, ENABLE), 1},
    // TX POWER Level Characteristic - Client Characteristic Configuration Descriptor
    [TX_POWER_SERVICE_IDX_POWER_LVL_NTF_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE), 2},
};


typedef struct
{
    ble_tx_power_callback callback;
    sibles_hdl handle;
    uint8_t state;
    uint8_t cccd_enable;
    uint8_t power_lvl;
} ble_tx_power_env_t;

static ble_tx_power_env_t g_tx_power_env_t;

static ble_tx_power_env_t *ble_tx_power_get_env(void)
{
    return &g_tx_power_env_t;
}

static uint8_t *ble_tx_power_get_cbk(uint8_t conn_idx, uint8_t idx, uint16_t *len)
{
    ble_tx_power_env_t *env = ble_tx_power_get_env();
    switch (idx)
    {
    case TX_POWER_SERVICE_IDX_POWER_LVL_VAL:
    {
        *len = sizeof(uint8_t);
        if (env->callback)
            env->power_lvl = env->callback(conn_idx, BLE_TX_POWER_SERVICE_SERVER_GET_POWER_LVL);
        LOG_I("power lvl %d", env->power_lvl);
        return &env->power_lvl;
        break;
    }
    default:
        break;
    }
    *len = 0;
    return NULL;
}

static uint8_t ble_tx_power_set_cbk(uint8_t conn_idx, sibles_set_cbk_t *para)
{
    ble_tx_power_env_t *env = ble_tx_power_get_env();
    switch (para->idx)
    {
    case TX_POWER_SERVICE_IDX_POWER_LVL_NTF_CFG:
    {
        LOG_I("tx power enable %d", *(para->value));
        env->cccd_enable = *(para->value);
        break;
    }
    default:
        break;
    }
    return 0;
}

int8_t ble_tx_power_level_notify_power_lvl(uint8_t conn_idx, uint8_t lvl)
{
    ble_tx_power_env_t *env = ble_tx_power_get_env();
    uint8_t ret = -1;
    if (env->state == TX_POWER_STATE_READY)
    {
        if (env->power_lvl != lvl)
        {
            env->power_lvl = lvl;
            sibles_value_t value;
            value.hdl = env->handle;
            value.idx = TX_POWER_SERVICE_IDX_POWER_LVL_VAL;
            value.len = sizeof(uint8_t);
            value.value = &env->power_lvl;
            int write_ret = sibles_write_value(conn_idx, &value);
            if (write_ret == value.len)
            {
                ret = 0;
            }
            else
            {
                LOG_W("power level write failed with %d", write_ret);
                ret = -3;
            }
        }
        else
        {
            LOG_D("power level the same %d, skip", lvl);
            ret = -2;
        }
    }
    else
    {
        LOG_I("power service state not ready %d", env->state);
    }
    return ret;
}



void ble_tx_power_service_server_init(ble_tx_power_callback callback, uint8_t power_lvl)
{
    ble_tx_power_env_t *env = ble_tx_power_get_env();

    if (env->state == TX_POWER_STATE_IDLE)
    {
        sibles_register_svc_t svc;

        svc.att_db = (struct attm_desc *)&tx_power_att_db;
        svc.num_entry = TX_POWER_SERVICE_IDX_NB;
        svc.sec_lvl = PERM(SVC_AUTH, NO_AUTH) |  PERM(SVC_MI, ENABLE);
        svc.uuid = ATT_SVC_TX_POWER;
        env->handle = sibles_register_svc(&svc);
        if (env->handle)
            sibles_register_cbk(env->handle, ble_tx_power_get_cbk, ble_tx_power_set_cbk);
        env->state = TX_POWER_STATE_READY;
    }
    env->power_lvl = power_lvl;
    env->callback = callback;
}

#endif