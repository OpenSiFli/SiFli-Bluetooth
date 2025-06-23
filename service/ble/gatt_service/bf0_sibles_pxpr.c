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
#include "bf0_ble_pxpr.h"
#include "bf0_ble_gap.h"
#include "bf0_ble_common.h"

#ifdef BSP_BLE_PXPR
#define LOG_TAG "pxp_srv"
#include "log.h"


/// LLS Handles offsets
enum
{
    LLS_IDX_SVC,

    LLS_IDX_ALERT_LVL_CHAR,
    LLS_IDX_ALERT_LVL_VAL,

    LLS_IDX_NB,
};



/// TXPS Handles offsets
enum
{
    TXPS_IDX_SVC,

    TXPS_IDX_TX_POWER_LVL_CHAR,
    TXPS_IDX_TX_POWER_LVL_VAL,

    TXPS_IDX_NB,
};





/// Full LLS Database Description - Used to add attributes into the database
const struct attm_desc proxr_lls_att_db[LLS_IDX_NB] =
{
    // Link Loss Service Declaration
    [LLS_IDX_SVC]                   =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},
    // Alert Level Characteristic Declaration
    [LLS_IDX_ALERT_LVL_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Alert Level Characteristic Value
    [LLS_IDX_ALERT_LVL_VAL]         =   {ATT_CHAR_ALERT_LEVEL, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE), sizeof(uint8_t)},
};


/// Full TXPS Database Description - Used to add attributes into the database
const struct attm_desc proxr_txps_att_db[TXPS_IDX_NB] =
{
    // TX Power Service Declaration
    [TXPS_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},
    // TX Power Level Characteristic Declaration
    [TXPS_IDX_TX_POWER_LVL_CHAR]       =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // TX Power Level Characteristic Value
    [TXPS_IDX_TX_POWER_LVL_VAL]     =   {ATT_CHAR_TX_POWER_LEVEL, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(uint8_t)},
};





typedef struct
{
    sibles_hdl lls_hdl;
    sibles_hdl ias_hdl;
    sibles_hdl txps_hdl;
    ble_pxpr_callback callback;
    uint8_t lls_level;
    int8_t tx_power_lvl;
} ble_pxpr_env_t;

static ble_pxpr_env_t g_pxpr_env;

static ble_pxpr_env_t *ble_pxpr_get_env(void)
{
    return &g_pxpr_env;
}


static uint8_t *ble_pxpr_lls_get_cbk(uint8_t conn_idx, uint8_t idx, uint16_t *len)
{
    ble_pxpr_env_t *env = ble_pxpr_get_env();

    switch (idx)
    {
    case LLS_IDX_ALERT_LVL_VAL:
    {
        *len = sizeof(uint8_t);
        LOG_I("pxpr get lls level %d\r\n", env->lls_level);
        return &env->lls_level;
        break;
    }
    default:
        break;
    }
    *len = 0;
    return NULL;
}

static uint8_t ble_pxpr_lls_set_cbk(uint8_t conn_idx, sibles_set_cbk_t *para)
{
    ble_pxpr_env_t *env = ble_pxpr_get_env();
    switch (para->idx)
    {
    case LLS_IDX_ALERT_LVL_VAL:
    {
        env->lls_level = *(para->value);
        LOG_I("lls set value %d\r\n", *(para->value));
        break;
    }
    default:
        break;
    }
    return 0;
}

static void ble_pxpr_ias_callback(uint8_t conn_idx, uint8_t alert_lvl)
{
    ble_pxpr_env_t *env = ble_pxpr_get_env();

    env->callback(conn_idx, BLE_PXPR_ALERT_IND, alert_lvl);
}

static uint8_t *ble_pxpr_txps_get_cbk(uint8_t conn_idx, uint8_t idx, uint16_t *len)
{
    ble_pxpr_env_t *env = ble_pxpr_get_env();
    switch (idx)
    {
    case TXPS_IDX_TX_POWER_LVL_VAL:
    {
        *len = sizeof(uint8_t);
        LOG_I("pxp pwoer %d", env->tx_power_lvl);
        return (uint8_t *)&env->tx_power_lvl;
        break;
    }
    default:
        break;
    }
    *len = 0;
    return NULL;
}


void ble_pxpr_init(ble_pxpr_callback callback)
{
    ble_pxpr_env_t *env = ble_pxpr_get_env();
    sibles_register_svc_t svc;

    // lls register
    svc.att_db = (struct attm_desc *)&proxr_lls_att_db;
    svc.num_entry = LLS_IDX_NB;
    svc.sec_lvl = PERM(SVC_AUTH, NO_AUTH);
    svc.uuid = ATT_SVC_LINK_LOSS;
    env->lls_hdl = sibles_register_svc(&svc);
    if (env->lls_hdl)
        sibles_register_cbk(env->lls_hdl, ble_pxpr_lls_get_cbk, ble_pxpr_lls_set_cbk);

    int8_t ret = ble_ias_init(ble_pxpr_ias_callback);
    //RT_ASSERT(ret >= 0);

    // lls register
    svc.att_db = (struct attm_desc *)&proxr_txps_att_db;
    svc.num_entry = TXPS_IDX_NB;
    svc.sec_lvl = PERM(SVC_AUTH, NO_AUTH);
    svc.uuid = ATT_SVC_TX_POWER;
    env->txps_hdl = sibles_register_svc(&svc);
    if (env->txps_hdl)
    {
        sibles_register_cbk(env->txps_hdl, ble_pxpr_txps_get_cbk, NULL);
        // Currently it not support due to legacy ADV option open but extended set
        ble_gap_get_local_adv_tx_power();
    }

    env->callback = callback;

}

int ble_pxpr_event_handler(uint16_t event_id, uint8_t *data, uint16_t len, uint32_t context)
{
    ble_pxpr_env_t *env = ble_pxpr_get_env();
    switch (event_id)
    {
    case BLE_GAP_LOCAL_ADV_TX_POWER_IND:
    {
        ble_gap_dev_adv_tx_power_ind_t *ind = (ble_gap_dev_adv_tx_power_ind_t *)data;
        LOG_I("pxp tx power %d\r\n", ind->power_lvl);
        env->tx_power_lvl = ind->power_lvl;
        break;
    }
    case BLE_GAP_DISCONNECTED_IND:
    {
        ble_gap_disconnected_ind_t *ind = (ble_gap_disconnected_ind_t *)data;
        LOG_I("pxp dis %d\r\n", ind->reason);
        if (ind->reason != LL_ERR_REMOTE_USER_TERM_CON)
        {
            if (env->callback)
                env->callback(ind->conn_idx, BLE_PXPR_LINK_LOSS_IND, env->lls_level);
        }
        break;
    }
    default:
        break;
    }
    return 0;
}


BLE_EVENT_REGISTER(ble_pxpr_event_handler, NULL);
#endif // BSP_BLE_PXPR


