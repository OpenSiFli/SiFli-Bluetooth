/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_BLE_FMPT_H
#define __BF0_BLE_FMPT_H

enum findt_alert_level
{
    FINDT_IAS_NO_ALERT,
    FINDT_IAS_MID_ALERT,
    FINDT_IAS_HIGH_ALERT,
};

typedef void (*ble_fmpt_callback)(uint8_t conn_idx, uint8_t alert_lvl);

void ble_fmpt_init(ble_fmpt_callback callback);


#endif //__BF0_BLE_FMPT_H


