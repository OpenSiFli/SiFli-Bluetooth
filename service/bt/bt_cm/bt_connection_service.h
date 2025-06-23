/*
 * SPDX-FileCopyrightText: 2024-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "bts2_app_inc.h"

// int8_t bt_connect_req(BTS2S_BD_ADDR *bd);
int8_t bt_cancel_connect_req(BTS2S_BD_ADDR *bd);
int8_t bt_disconnect_req(BTS2S_BD_ADDR *bd);

#ifdef __cplusplus
}
#endif