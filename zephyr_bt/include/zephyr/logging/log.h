/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_LOGGING_LOG_H_
#define ZEPHYR_INCLUDE_LOGGING_LOG_H_

#include <rtt4zephyr.h>
/*
 * Eclipse CDT or JetBrains Clion parser is sometimes confused by logging API
 * code and freezes the whole IDE. Following lines hides LOG_x macros from them.
 */
#undef LOG_ERR
#undef LOG_WRN
#undef LOG_INF
#undef LOG_DBG

#undef LOG_HEXDUMP_ERR
#undef LOG_HEXDUMP_WRN
#undef LOG_HEXDUMP_INF
#undef LOG_HEXDUMP_DBG

#define LOG_ERR(...) (void) 0
#define LOG_WRN(...) (void) 0
#define LOG_DBG(...) (void) 0
#define LOG_INF(...) (void) 0

#define LOG_HEXDUMP_ERR(...) (void) 0
#define LOG_HEXDUMP_WRN(...) (void) 0
#define LOG_HEXDUMP_DBG(...) (void) 0
#define LOG_HEXDUMP_INF(...) (void) 0

#define LOG_MODULE_REGISTER(...)

#if 0
    #include <log.h>
    #undef LOG_DBG
    #define LOG_DBG LOG_D
    #undef LOG_WRN
    #define LOG_WRN LOG_W
    #undef LOG_ERR
    #define LOG_ERR LOG_E
    #undef LOG_INF
    #define LOG_INF LOG_I
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_LOGGING_LOG_H_ */
