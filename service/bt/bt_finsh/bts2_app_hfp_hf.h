/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_HFP_HF_H_
#define _BTS2_APP_HFP_HF_H_

#include "drivers/bt_device.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef CFG_HFP_HF
void bt_hfp_hf_init(bts2_app_stru *bts2_app_data);
bt_err_t bt_hfp_hf_start_enb(bts2_app_stru *bts2_app_data);
bt_err_t bt_hfp_hf_start_disb(bts2_app_stru *bts2_app_data);
void bt_hfp_hf_clean_flag();
U8 bt_hfp_hf_get_ring_type(void);
bt_err_t bt_hfp_hf_connect_request(BTS2S_BD_ADDR *bd);
bt_err_t bt_hfp_hf_start_disc(BTS2S_BD_ADDR *bd);
bt_err_t bt_hfp_hf_audio_transfer(U8 type);
bt_err_t bt_hfp_hf_voice_recog_send(U8 active);
bt_err_t bt_hfp_hf_dial_by_mem_send(U16 memory);
bt_err_t bt_hfp_hf_last_num_dial_send(void);
bt_err_t bt_hfp_hf_make_call_by_number_send(U8 *payload, U8 payload_len);
bt_err_t bt_hfp_hf_answer_call_send(void);
bt_err_t bt_hfp_hf_hangup_call_send(void);
bt_err_t bt_hfp_hf_update_spk_vol(U8 vol);
bt_err_t bt_hfp_hf_update_mic_vol(U8 vol);
bt_err_t bt_hfp_hf_at_btrh_query_send(void);
bt_err_t bt_hfp_hf_at_btrh_cmd_send(U8 mode);
bt_err_t bt_hfp_hf_at_binp_send(void);
bt_err_t bt_hfp_hf_at_clip_send(U8 enable);
bt_err_t bt_hfp_hf_at_cmee_send(BOOL val);
bt_err_t bt_hfp_hf_at_cnum_send(void);
bt_err_t bt_hfp_hf_at_ccwa_send(BOOL val);
bt_err_t bt_hfp_hf_at_chld_send(U8 *payload, U8 payload_len);
bt_err_t bt_hfp_hf_at_clcc_send(void);
bt_err_t bt_hfp_hf_at_cops_cmd_send(void);
bt_err_t bt_hfp_hf_at_dtmf_send(char key);
bt_err_t bt_hfp_hf_at_nrec_send(void);
bt_err_t bt_hfp_hf_update_batt_send(U8 batt_val);

void bt_hfp_hf_rfc_conn_accept_hdl(void);
void bt_hfp_hf_rfc_conn_rej_hdl(void);
U8 bt_hfp_hf_get_ciev_info(BTS2S_HF_CIEV_IND *msg);
void bt_hfp_hf_msg_hdl(bts2_app_stru *bts2_app_data);

#else
#define bt_hfp_hf_start_enb(bts2_app_data)  (-BT_ERROR_INPARAM)
#define bt_hfp_hf_start_disb(bts2_app_data) (-BT_ERROR_INPARAM)
#define bt_hfp_hf_clean_flag()
#define bt_hfp_hf_get_ring_type()           0
#define bt_hfp_hf_connect_request(bd)      (-BT_ERROR_INPARAM)
#define bt_hfp_hf_start_disc(bd)            (-BT_ERROR_INPARAM)
#define bt_hfp_hf_audio_transfer(type)      (-BT_ERROR_INPARAM)
#define bt_hfp_hf_voice_recog_send(active)  (-BT_ERROR_INPARAM)
#define bt_hfp_hf_dial_by_mem_send(memory)  (-BT_ERROR_INPARAM)
#define bt_hfp_hf_last_num_dial_send()      (-BT_ERROR_INPARAM)
#define bt_hfp_hf_make_call_by_number_send(payload,payload_len) (-BT_ERROR_INPARAM)
#define bt_hfp_hf_answer_call_send()        (-BT_ERROR_INPARAM)
#define bt_hfp_hf_hangup_call_send()        (-BT_ERROR_INPARAM)
#define bt_hfp_hf_update_spk_vol(vol)       (-BT_ERROR_INPARAM)
#define bt_hfp_hf_update_mic_vol(vol)       (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_btrh_query_send()      (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_btrh_cmd_send(mode)    (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_binp_send()            (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_clip_send(enable)      (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_cmee_send(val)         (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_cnum_send()            (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_ccwa_send(val)         (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_chld_send(payload,payload_len) (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_clcc_send()            (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_cops_cmd_send()        (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_dtmf_send(key)         (-BT_ERROR_INPARAM)
#define bt_hfp_hf_at_nrec_send()            (-BT_ERROR_INPARAM)
#define bt_hfp_hf_update_batt_send(batt_val)    (-BT_ERROR_INPARAM)
#define bt_hfp_hf_connect_request(bd)      (-BT_ERROR_INPARAM)

#endif




#ifdef __cplusplus
}
#endif
#endif

