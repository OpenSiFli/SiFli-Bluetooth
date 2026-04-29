/*
 * SPDX-FileCopyrightText: 2026-2026 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * ZBT mode stub implementations
 * These stubs provide bt_mem_alloc and sifli_hci_log_get_enable
 * to avoid dependency on lib_bt_gcc.a
 * Uses weak linkage to allow override by stronger definitions
 */


#include <rtthread.h>
#include "bf0_sibles_nvds.h"
#include "ble_stack.h"

#define LOG_TAG "utils_zbt"
#include "log.h"

#ifdef DISABLE_SF_BT_LIB
/**
 * @brief BLE stack boot function (ZBT mode stub)
 * In ZBT mode, this is a no-op because the BT Controller is managed
 * by the Zephyr BT stack via HCI
 */
void ble_boot(sifli_msg_func_t callback) __attribute__((weak));
void ble_boot(sifli_msg_func_t callback)
{
    LOG_I("ble_boot called (ZBT mode - no-op)\n");
    (void)callback;
}

/**
 * @brief Memory allocation for BLE stack (ZBT mode stub)
 * Uses rtthread's memory allocator instead of lib_bt_gcc.a
 */
void *bt_mem_alloc(uint32_t size) __attribute__((weak));
void *bt_mem_alloc(uint32_t size)
{
    return rt_malloc(size);
}

/**
 * @brief Memory free for BLE stack (ZBT mode stub)
 */
void bt_mem_free(void *ptr) __attribute__((weak));
void bt_mem_free(void *ptr)
{
    rt_free(ptr);
}

/**
 * @brief Check if HCI log is enabled (ZBT mode stub)
 */
bool sifli_hci_log_get_enable(void) __attribute__((weak));
bool sifli_hci_log_get_enable(void)
{
    return false;
}

/**
 * @brief Assert handler for BLE stack (ZBT mode stub)
 * Called by BT_OOM_ASSERT and other assert macros when RT_DEBUG is enabled
 */
void rw_assert_handler(const char *ex, const char *func, size_t line) __attribute__((weak));
void rw_assert_handler(const char *ex, const char *func, size_t line)
{
    OS_ASSERT(ex);
}
#endif

