/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef BSP_BLE_PXPR

#ifndef __BF0_BLE_PXPR_H
#define __BF0_BLE_PXPR_H

enum ble_pxpr_event_t
{
    BLE_PXPR_LINK_LOSS_IND,
    BLE_PXPR_ALERT_IND,
};

typedef void (*ble_pxpr_callback)(uint8_t conn_idx, uint8_t event, uint8_t alert_lvl);

void ble_pxpr_init(ble_pxpr_callback callback);

#endif // __BF0_BLE_PXPR_H

#endif // BSP_BLE_PXPR


