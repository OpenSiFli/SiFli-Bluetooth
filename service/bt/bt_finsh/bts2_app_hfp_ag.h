/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_HFP_AG_H_
#define _BTS2_APP_HFP_AG_H_




#ifdef __cplusplus
extern "C" {
#endif

#include "bts2_global.h"
#ifdef CFG_HFP_AG

/*************************************func *************************************/
void bt_hfp_ag_app_init(bts2_app_stru *bts2_app_data);
void bt_hfp_ag_msg_hdl(bts2_app_stru *bts2_app_data);
void bt_hfp_start_profile_service(bts2_app_stru *bts2_app_data);
void bt_hfp_stop_profile_service(bts2_app_stru *bts2_app_data);
int bt_hfp_ag_connect_request(BTS2S_BD_ADDR *bd);
void bt_hfp_ag_disconnect_request(BTS2S_BD_ADDR *bd);
void bt_hfp_connect_audio(BTS2S_BD_ADDR *bd);
void bt_hfp_disconnect_audio(BTS2S_BD_ADDR *bd);
void bt_hfp_ag_call_state_update_listener(U8 mux_id, HFP_CALL_INFO_T *call_info);
void bt_hfp_ag_remote_calls_res_hdl(U8 mux_id, hfp_remote_calls_info_t *call_info);
void bt_hfp_ag_app_call_status_change(U8 mux_id, char *phone_num, uint8_t phone_len, uint8_t active, uint8_t callsetup_state, uint8_t call_dir);
void bt_hfp_ag_app_cind_status_change(uint8_t type, uint8_t val);
hfp_phone_call_info_t *bt_hfp_ag_app_get_remote_call_info();



/*************************************AT CMD *************************************/
void bt_hfp_ag_spk_vol_control(U8 mux_id, U8 vol);
void bt_hfp_ag_mic_vol_control(U8 mux_id, U8 vol);
void bt_hfp_ag_cind_response(U8 mux_id, hfp_cind_status_t *cind_status);
void bt_hfp_ag_ind_status_update(U8 mux_id, U8 type, U8 val);
void bt_hfp_ag_brva_response(U8 mux_id, U8 val);
void bt_hfp_ag_set_inband(U8 mux_id, U8 val);
void bt_hfp_ag_cnum_response(U8 mux_id, hfp_phone_number_t *local_phone_num);
void bt_hfp_ag_set_btrh(U8 mux_id, U8 val);
void bt_hfp_ag_clcc_response(U8 mux_id, hfp_phone_call_info_t *call_info);
void bt_hfp_ag_cops_response(U8 mux_id, char *payload, U8 payload_len);
void bt_hfp_ag_set_bcs(U8 mux_id, U8 code_id);
void bt_hfp_ag_clip_response(U8 mux_id, hfp_phone_number_t *remote_phone_num);
void bt_hfp_ag_at_result_res(U8 mux_id, U8 res);

#endif

#endif
#ifdef __cplusplus
}
#endif



