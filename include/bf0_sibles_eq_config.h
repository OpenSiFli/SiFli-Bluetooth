/*
 * SPDX-FileCopyrightText: 2023-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BF0_SIBLES_EQ_CONFIG_H_
#define BF0_SIBLES_EQ_CONFIG_H_

#include <stdint.h>
#include <stdlib.h>
#include "bf0_ble_common.h"

typedef uint32_t eq_state_t;

enum
{
    BLE_EQ_TYPE_CALL = 0,
    BLE_EQ_TYPE_MUSIC = 1,
};

eq_state_t ble_eq_set_adc(int8_t val);
eq_state_t ble_eq_get_adc(void);
eq_state_t ble_eq_set_dac(uint8_t type, int8_t val);
eq_state_t ble_eq_get_dac(uint8_t type);
eq_state_t ble_eq_set_max_dac_level(uint8_t type, int8_t val);
eq_state_t ble_eq_get_max_dac_level(uint8_t type);
eq_state_t ble_eq_set_volume_lvl_via_idx(uint8_t type, uint8_t idx, int8_t val);
eq_state_t ble_eq_get_volume_lvl_via_idx(uint8_t type, uint8_t idx);
eq_state_t ble_eq_set_default_volume(uint8_t type, uint8_t idx, int8_t val);
eq_state_t ble_eq_get_version(void);
eq_state_t ble_eq_set_parameter(uint8_t type, uint8_t idx, uint8_t *para, uint8_t len);
eq_state_t ble_eq_get_parameter(uint8_t type, uint8_t idx);
eq_state_t ble_eq_set_state(uint8_t type, uint8_t state);
eq_state_t ble_eq_start(uint8_t is_start);
void ble_eq_register(uint8_t conn_idx);



#endif //BF0_SIBLES_EQ_CONFIG_H_


