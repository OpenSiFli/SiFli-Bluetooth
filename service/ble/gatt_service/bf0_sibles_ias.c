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
#include "bf0_sibles_ias.h"

#define LOG_TAG "ias_srv"
#include "log.h"


/// IAS Handles offsets
enum
{
    IAS_IDX_SVC,

    IAS_IDX_ALERT_LVL_CHAR,
    IAS_IDX_ALERT_LVL_VAL,

    IAS_IDX_NB,
};

typedef enum
{
    IAS_STATE_IDLE,
    IAS_STATE_READY
} ble_ias_state_t;


/// Full IAS Database Description - Used to add attributes into the database
const struct attm_desc ias_att_db[IAS_IDX_NB] =
{
    // Immediate Alert Service Declaration
    [IAS_IDX_SVC]                   =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},
    // Alert Level Characteristic Declaration
    [IAS_IDX_ALERT_LVL_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Alert Level Characteristic Value
    [IAS_IDX_ALERT_LVL_VAL]         =   {ATT_CHAR_ALERT_LEVEL, PERM(WRITE_COMMAND, ENABLE), PERM(RI, ENABLE), sizeof(uint8_t)},
};


#define MAX_IAS_USER 3
typedef struct
{
    ble_ias_state_t state;
    ble_ias_callback callback[MAX_IAS_USER];
    sibles_hdl handle;
} ble_ias_env_t;


static ble_ias_env_t g_ias_env;


static ble_ias_env_t *ble_ias_get_env(void)
{
    return &g_ias_env;
}

static uint8_t ble_ias_set_cbk(uint8_t conn_idx, sibles_set_cbk_t *para)
{
    ble_ias_env_t *env = ble_ias_get_env();
    switch (para->idx)
    {
    case IAS_IDX_ALERT_LVL_VAL:
    {
        uint32_t i;
        for (i = 0; i < MAX_IAS_USER; i++)
        {
            LOG_I("callback %x\r\n", env->callback[i]);
            if (env->callback[i])
                env->callback[i](conn_idx, *(para->value));
        }
        break;
    }
    default:
        break;
    }
    return 0;
}

static int8_t ble_ias_register_callback(ble_ias_callback callback)
{
    int8_t ret = 0;
    uint32_t i;
    ble_ias_env_t *env = ble_ias_get_env();
    for (i = 0; i < MAX_IAS_USER; i++)
    {
        if (env->callback[i] == NULL)
        {
            env->callback[i] = callback;
            break;
        }
    }
    if (i == MAX_IAS_USER)
        ret = -1;
    return ret;
}

int8_t ble_ias_init(ble_ias_callback callback)
{
    ble_ias_env_t *env = ble_ias_get_env();

    if (env->state == IAS_STATE_IDLE)
    {
        sibles_register_svc_t svc;

        svc.att_db = (struct attm_desc *)&ias_att_db;
        svc.num_entry = IAS_IDX_NB;
        svc.sec_lvl = PERM(SVC_AUTH, NO_AUTH);
        svc.uuid = ATT_SVC_IMMEDIATE_ALERT;
        env->handle = sibles_register_svc(&svc);
        if (env->handle)
            sibles_register_cbk(env->handle, NULL, ble_ias_set_cbk);
        env->state = IAS_STATE_READY;
    }
    return ble_ias_register_callback(callback);
}




