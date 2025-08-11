/*
 * SPDX-FileCopyrightText: 2025-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_BLE_TX_POWER_SERVICE_SERVER_H
#define __BF0_BLE_TX_POWER_SERVICE_SERVER_H


enum bf0_tx_power_event_t
{
    BLE_TX_POWER_SERVICE_SERVER_GET_POWER_LVL,
};


typedef uint8_t (*ble_tx_power_callback)(uint8_t conn_idx, uint8_t event);

int8_t ble_tx_power_level_notify_power_lvl(uint8_t conn_idx, uint8_t lvl);

void ble_tx_power_service_server_init(ble_tx_power_callback callback, uint8_t power_lvl);


#endif //__BF0_BLE_TX_POWER_SERVICE_SERVER_H


