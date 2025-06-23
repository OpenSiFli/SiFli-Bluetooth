/*
 * SPDX-FileCopyrightText: 2022-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BLUETOOTH_INT_H
#define __BLUETOOTH_INT_H

extern void rom_config_set_default_xtal_enabled(uint8_t default_xtal_enabled);
extern void rom_config_set_default_rc_cycle(uint8_t default_rc_cycle);
extern void rom_config_set_lld_prog_delay(uint8_t lld_prog_delay);
extern void     ble_standby_sleep_after_handler_rom();
extern int32_t ble_standby_sleep_pre_handler_rom();
extern void bluetooth_idle_hook_func(void);
extern int bluetooth_stack_suspend(void);
extern int32_t ble_standby_sleep_pre_handler_rom();
extern void     ble_standby_sleep_after_handler_rom();
extern void rf_ptc_config(uint8_t is_reset);
extern void bluetooth_config(void);
extern void wlan_coex_config(void);
extern void pta_io_config(void);

void *bt_mem_alloc(size_t size);
void *bt_mem_calloc(size_t count, size_t nbytes);
void bt_mem_free(void *ptr);




#endif //__BLUETOOTH_INT_H


