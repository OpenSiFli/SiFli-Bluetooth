/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_BT_GATT_H_
#define _BTS2_APP_BT_GATT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bts2_global.h"
#ifdef CFG_BR_GATT_SRV
#include "bts2_app_demo.h"

/*************************************func *************************************/
void bt_bt_gatt_msg_hdl(bts2_app_stru *bts2_app_data);

#endif
#endif
#ifdef __cplusplus
}
#endif



