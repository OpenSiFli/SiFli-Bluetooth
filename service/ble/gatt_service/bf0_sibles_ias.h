/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_SIBLES_IAS_H
#define __BF0_SIBLES_IAS_H

typedef void (*ble_ias_callback)(uint8_t conn_idx, uint8_t alert_lvl);

int8_t ble_ias_init(ble_ias_callback callback);

#endif // __BF0_SIBLES_IAS_H





