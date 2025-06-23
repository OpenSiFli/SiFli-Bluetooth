/*
 * SPDX-FileCopyrightText: 2024-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bt_connection_service.h"
// int8_t bt_connect_req(BTS2S_BD_ADDR *bd)
// {

// }

int8_t bt_cancel_connect_req(BTS2S_BD_ADDR *bd)
{
    return gap_cancel_connect_req(bd);
}

int8_t bt_disconnect_req(BTS2S_BD_ADDR *bd)
{
    return gap_disconnect_req(bd);
}