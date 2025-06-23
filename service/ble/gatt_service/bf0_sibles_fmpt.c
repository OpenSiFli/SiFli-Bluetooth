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
#include "bf0_ble_fmpt.h"


typedef struct
{
    ble_fmpt_callback callback;
} ble_fmpt_env_t;


static ble_fmpt_env_t g_findt_env;

static ble_fmpt_env_t *ble_fmpt_get_env(void)
{
    return &g_findt_env;
}

static void ble_fmpt_set_cbk(uint8_t conn_idx, uint8_t alert_lvl)
{
    ble_fmpt_env_t *env = ble_fmpt_get_env();

    env->callback(conn_idx, alert_lvl);
}

void ble_fmpt_init(ble_fmpt_callback callback)
{
    ble_fmpt_env_t *env = ble_fmpt_get_env();
    int8_t ret = ble_ias_init(ble_fmpt_set_cbk);
    //RT_ASSERT(ret >= 0);
    env->callback = callback;
}




