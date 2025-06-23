/*
 * SPDX-FileCopyrightText: 2024-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_BT_DIS_H_
#define _BTS2_APP_BT_DIS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#ifdef CFG_BT_DIS
#include "bts2_app_demo.h"
#include "bt_dis_api.h"
#include "drivers/bt_device.h"

/*************************************func *************************************/
void bt_dis_app_set_prim_rec(uint8_t prim_rec);
void bt_dis_app_set_vendor_id(uint16_t vend_id, uint16_t vend_id_source);
void bt_dis_app_set_prod_id(uint16_t prod_id);
void bt_dis_app_set_prod_ver(uint16_t prod_ver);
void bt_dis_app_set_spec_id(uint16_t spec_id);

bt_dis_device_info_t *bt_dis_app_get_dis_info();
uint8_t bt_dis_app_get_prim_rec();
uint16_t bt_dis_app_get_vendor_id();
uint16_t bt_dis_app_get_vend_id_source();
uint16_t bt_dis_app_get_prod_id();
uint16_t bt_dis_app_get_prod_ver();
uint16_t bt_dis_app_get_spec_id();

void bt_dis_app_msg_hdl(bts2_app_stru *bts2_app_data);

bt_err_t bt_dis_app_sdp_reg();
bt_err_t bt_dis_app_sdp_unreg();

#endif
#endif
#ifdef __cplusplus
}
#endif

