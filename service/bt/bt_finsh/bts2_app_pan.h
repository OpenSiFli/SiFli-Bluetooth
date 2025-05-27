/*
*********************************************************************************************************
* Copyright (C) 2006-2021 Lianway Corporation
*
* Introduction:
*       The purpose of design is to provide PAN app.
*
* File : bts2_app_pan.h
*
* History:
*
*
*********************************************************************************************************
*/

#ifndef _BTS2_APP_PAN_H_
#define _BTS2_APP_PAN_H_

#include "bts2_app_demo.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BT_FINSH_PAN

void bt_pan_init(bts2_app_stru *bts2_app_data);
void bt_pan_reg(bts2_app_stru *bts2_app_data);
void bt_pan_enable(bts2_app_stru *bts2_app_data);
int bt_pan_conn(BTS2S_BD_ADDR *remote_addr);
void bt_pan_update_addr(BTS2S_BD_ADDR *bd_addr);
void bt_pan_disc(BTS2S_BD_ADDR *bd);
void bt_hdl_pan_msg(bts2_app_stru *bts2_app_data);

void bt_pan_set_ip_addr(char *string);
void bt_pan_set_netmask(char *string);
void bt_pan_set_gw(char *string);
void bt_pan_set_dns(char *string1, char *string2);
void bt_pan_scan_proc_net_dev(void);
void bt_pan_set_nap_route(char *string);
void bt_pan_set_dns1(char *string);
void bt_pan_set_dns2(char *string);
void bt_pan_send_data(void *buff, int len);
//extern BTS2S_ETHER_ADDR bt_pan_get_remote_mac_address(struct rt_bt_lwip_pan_dev *bt_dev);
extern BTS2S_ETHER_ADDR bt_pan_get_mac_address();
extern BTS2S_ETHER_ADDR bconvbd2etherbig(BTS2S_BD_ADDR *bd);
extern void pan_reg_req(U16 conn_hdl, U16 data_hdl, BTS2S_BD_ADDR local_bd);





#endif

#ifdef __cplusplus
}
#endif
#endif
