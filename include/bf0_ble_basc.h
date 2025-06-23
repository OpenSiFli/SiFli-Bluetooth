/*
 * SPDX-FileCopyrightText: 2024-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_BLE_BASC_H
#define __BF0_BLE_BASC_H

#include "bf0_ble_common.h"


enum ble_basc_event
{
    //Remote Battry Level Notification see @ble_basc_bat_lev_t
    BLE_BASC_BAT_LEV_NOTIFY = BLE_BAS_TYPE,
    //Read remote Battry Level response see @ble_basc_bat_lev_t
    BLE_BASC_READ_BAT_LEV_RSP,
};


/**
 * @brief The structure of BLE_BASC_BAT_LEV_NOTIFY and BLE_BASC_READ_BAT_LEV_RSP.
 */
typedef struct
{
    // battery level
    uint8_t lev;
} ble_basc_bat_lev_t;


/**
 * @brief initiate battery service
   @param[in] is_enable whether enabled.
 */
void ble_basc_init(uint8_t enable);


/**
 * @brief Enable CCCD notify of battery level.
   @param[in] conn_idx connection index.
   @param[in] is_enable whether enabled.
   @retval result SIBLES_WRITE_NO_ERR is successful.
 */
int8_t ble_basc_enable_bat_lev_notify(uint8_t conn_idx, uint8_t enable);


/**
 * @brief read remote battery level.
   @param[in] conn_idx connection index.
   @retval result 0 is successful.
 */
int8_t ble_basc_read_battery_level(uint8_t conn_idx);


/**
 * @brief search remote battery service .
   @param[in] conn_idx connection index.
   @retval result 0 is successful.
 */
int8_t ble_basc_enable(uint8_t conn_idx);

#endif //__BF0_BLE_BASC_H