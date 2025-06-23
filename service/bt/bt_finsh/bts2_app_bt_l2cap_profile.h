/*
 * SPDX-FileCopyrightText: 2024-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_BT_L2CAP_PROFILE_H_
#define _BTS2_APP_BT_L2CAP_PROFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bts2_global.h"
#ifdef CFG_BT_L2CAP_PROFILE
#include "bts2_app_demo.h"

#define BT_SYNC_3SCO_PACKET_TYPE              0x0380  // only 2EV3
#define BT_SYNC_3SCO_TX_BANDWIDTH             (8000)
#define BT_SYNC_3SCO_RX_BANDWIDTH             (8000)
#define BT_SYNC_3SCO_VOICE_SETTING            (BT_INPUT_CODING_LINEAR | BT_INPUT_DATA_FMT_2COMPLEMENT | BT_INPUT_SAMPLE_SIZE_16BIT | BT_AIR_CODING_CVSD)
#define BT_SYNC_3SCO_MAX_LATENCY              (0xFFFF)//Don't care
#define BT_SYNC_3SCO_RE_TX_EFFORT             (0x00)//no retansmission, 3sco don't support retansmission

void bt_l2cap_profile_app_init_data(bts2_app_stru *bts2_app_data);
void bt_l2cap_profile_app_msg_hdl(bts2_app_stru *bts2_app_data);
void bt_l2cap_profile_hci_msg_hdl(bts2_app_stru *bts2_app_data);
/*************************************BT_L2CAP PROFILE CMD *************************************/
U8 bt_l2cap_profile_app_reg_service(U16 psm, U16 flag, U16 flush_timeout);
U8 bt_l2cap_profile_app_unreg_service(U16 psm);
U8 bt_l2cap_profile_app_connect_req(BTS2S_BD_ADDR *bd, U16 local_psm, U16 remote_psm);
void bt_l2cap_profile_app_conn_ind(BTS2S_BT_L2CAP_CONN_IND *conn_info);
U8 bt_l2cap_profile_app_disconnect_req(BTS2S_BD_ADDR *bd, U16 psm);
U8 bt_l2cap_profile_app_send_data_req(U16 cid, U16 payload_len, char *payload);
/*************************************BT_L2CAP PROFILE CMD *************************************/

/*************************************BT_L2CAP PROFILE SCO start *************************************/
U8 bt_l2cap_profile_app_connect_sco_req(BTS2S_BD_ADDR *bd);
U8 bt_l2cap_profile_app_disconnect_sco_req(BTS2S_BD_ADDR *bd);
U8 bt_l2cap_profile_app_sco_request_res(BTS2S_BD_ADDR *bd, U8 acpt);
/*************************************BT_L2CAP PROFILE SCO end *************************************/

#endif
#endif

#ifdef __cplusplus
}
#endif



