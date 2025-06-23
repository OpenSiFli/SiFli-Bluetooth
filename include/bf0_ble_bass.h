/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_BLE_BASS_H
#define __BF0_BLE_BASS_H


enum bf0_bass_event_t
{
    BLE_BASS_GET_BATTERY_LVL,
};


typedef uint8_t (*ble_bass_callback)(uint8_t conn_idx, uint8_t event);

int8_t ble_bass_notify_battery_lvl(uint8_t conn_idx, uint8_t lvl);

void ble_bass_init(ble_bass_callback callback, uint8_t battery_lvl);


#endif //__BF0_BLE_BASS_H


