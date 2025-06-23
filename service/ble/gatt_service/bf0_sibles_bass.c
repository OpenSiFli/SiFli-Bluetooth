/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "bf0_sibles.h"
#include "bf0_ble_bass.h"

#define LOG_TAG "ble_bass"
#include "log.h"


/// Battery Service Attributes Indexes
enum
{
    BAS_IDX_SVC,

    BAS_IDX_BATT_LVL_CHAR,
    BAS_IDX_BATT_LVL_VAL,
    BAS_IDX_BATT_LVL_NTF_CFG,

    BAS_IDX_NB,
};

typedef enum
{
    BASS_STATE_IDLE,
    BASS_STATE_READY,
    BASS_STATE_BUSY,
} ble_bass_state_t;



/// Full BAS Database Description - Used to add attributes into the database
const struct attm_desc bas_att_db[BAS_IDX_NB] =
{
    // Battery Service Declaration
    [BAS_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},

    // Battery Level Characteristic Declaration
    [BAS_IDX_BATT_LVL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
    // Battery Level Characteristic Value
    [BAS_IDX_BATT_LVL_VAL]         =   {ATT_CHAR_BATTERY_LEVEL,    PERM(RD, ENABLE) | PERM(NTF, ENABLE), PERM(RI, ENABLE), 0},
    // Battery Level Characteristic - Client Characteristic Configuration Descriptor
    [BAS_IDX_BATT_LVL_NTF_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},
};


typedef struct
{
    ble_bass_callback callback;
    sibles_hdl handle;
    uint8_t state;
    uint8_t cccd_enable;
    uint8_t bas_lvl;
} ble_bass_env_t;

static ble_bass_env_t g_bass_env_t;

static ble_bass_env_t *ble_bass_get_env(void)
{
    return &g_bass_env_t;
}

static uint8_t *ble_bass_get_cbk(uint8_t conn_idx, uint8_t idx, uint16_t *len)
{
    ble_bass_env_t *env = ble_bass_get_env();
    switch (idx)
    {
    case BAS_IDX_BATT_LVL_VAL:
    {
        *len = sizeof(uint8_t);
        if (env->callback)
            env->bas_lvl = env->callback(conn_idx, BLE_BASS_GET_BATTERY_LVL);
        LOG_I("battery lvl %d", env->bas_lvl);
        return &env->bas_lvl;
        break;
    }
    default:
        break;
    }
    *len = 0;
    return NULL;
}

static uint8_t ble_bass_set_cbk(uint8_t conn_idx, sibles_set_cbk_t *para)
{
    ble_bass_env_t *env = ble_bass_get_env();
    switch (para->idx)
    {
    case BAS_IDX_BATT_LVL_NTF_CFG:
    {
        LOG_I("bas enable %d", *(para->value));
        env->cccd_enable = *(para->value);
        break;
    }
    default:
        break;
    }
    return 0;
}

int8_t ble_bass_notify_battery_lvl(uint8_t conn_idx, uint8_t lvl)
{
    ble_bass_env_t *env = ble_bass_get_env();
    uint8_t ret = -1;
    if (env->state == BASS_STATE_READY)
    {
        if (env->bas_lvl != lvl)
        {
            env->bas_lvl = lvl;
            sibles_value_t value;
            value.hdl = env->handle;
            value.idx = BAS_IDX_BATT_LVL_VAL;
            value.len = sizeof(uint8_t);
            value.value = &env->bas_lvl;
            int write_ret = sibles_write_value(conn_idx, &value);
            if (write_ret == value.len)
            {
                ret = 0;
            }
            else
            {
                LOG_W("battery level write failed with %d", write_ret);
                ret = -3;
            }
        }
        else
        {
            LOG_D("battery level the same %d, skip", lvl);
            ret = -2;
        }
    }
    else
    {
        LOG_I("battery service state not ready %d", env->state);
    }
    return ret;
}



void ble_bass_init(ble_bass_callback callback, uint8_t battery_lvl)
{
    ble_bass_env_t *env = ble_bass_get_env();

    if (env->state == BASS_STATE_IDLE)
    {
        sibles_register_svc_t svc;

        svc.att_db = (struct attm_desc *)&bas_att_db;
        svc.num_entry = BAS_IDX_NB;
        svc.sec_lvl = PERM(SVC_AUTH, NO_AUTH);
        svc.uuid = ATT_SVC_BATTERY_SERVICE;
        env->handle = sibles_register_svc(&svc);
        if (env->handle)
            sibles_register_cbk(env->handle, ble_bass_get_cbk, ble_bass_set_cbk);
        env->state = BASS_STATE_READY;
    }
    env->bas_lvl = battery_lvl;
    env->callback = callback;
}




