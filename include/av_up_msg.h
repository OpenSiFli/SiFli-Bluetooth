/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AV_UPMSG_H_
#define _AV_UPMSG_H_


#ifdef __cplusplus
extern "C" {
#endif

void avs_av_secu_ind(BTS2S_AV_INST_DATA *clt_data);
void avs_av_secu_cfm_rej(BTS2S_AV_INST_DATA *clt_data);
void avs_av_secu_cfm_acpt(BTS2S_AV_INST_DATA *clt_data);

void avs_av_abort_ind(BTS2S_AV_INST_DATA *clt_data);
void avs_av_abort_cfm_acpt(BTS2S_AV_INST_DATA *clt_data);

void avs_av_close_ind(BTS2S_AV_INST_DATA *clt_data);
void avs_av_close_cfm_rej(BTS2S_AV_INST_DATA *clt_data);
void avs_av_close_cfm_acpt(BTS2S_AV_INST_DATA *clt_data);



void avs_av_suspend_ind(BTS2S_AV_INST_DATA *clt_data);
void avs_av_suspend_cfm_rej(BTS2S_AV_INST_DATA *clt_data);
void avs_av_suspend_cfm_acpt(BTS2S_AV_INST_DATA *clt_data);

void avs_av_start_ind(BTS2S_AV_INST_DATA *clt_data);
void avs_av_start_cfm_rej(BTS2S_AV_INST_DATA *clt_data);
void avs_av_start_cfm_acpt(BTS2S_AV_INST_DATA *clt_data);

void avs_av_open_ind(BTS2S_AV_INST_DATA *clt_data);
void avs_av_open_cfm_rej(BTS2S_AV_INST_DATA *clt_data);
void avs_av_open_cfm_acpt(BTS2S_AV_INST_DATA *clt_data);

void avs_av_recfgure_ind(BTS2S_AV_INST_DATA *clt_data);
void avs_av_recfgure_cfm_rej(BTS2S_AV_INST_DATA *clt_data);
void avs_av_recfgure_cfm_acpt(BTS2S_AV_INST_DATA *clt_data);

void avs_av_get_cfguration_ind(BTS2S_AV_INST_DATA *clt_data);
void avs_av_get_cfguration_cfm_rej(BTS2S_AV_INST_DATA *clt_data);
void avs_av_get_cfguration_cfm_acpt(BTS2S_AV_INST_DATA *clt_data);

void avs_av_set_cfguration_ind(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_set_cfguration_cfm_rej(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_set_cfguration_cfm_acpt(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);

void avs_av_get_capabilities_ind(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_get_capabilities_cfm_rej(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_get_capabilities_cfm_acpt(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);

void avs_av_discover_ind(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_discover_cfm_rej(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_discover_cfm_acpt(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);


void avs_av_close_srch_ind(BTS2S_AV_INST_DATA *clt_data);


void avs_av_stream_data_ind(BTS2S_AV_INST_DATA *clt_data);

void avs_av_get_all_capabilities_cfm_acpt(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_get_all_capabilities_cfm_rej(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_get_all_capabilities_ind(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_delay_report_cfm_acpt(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_delay_report_cfm_rej(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);
void avs_av_delay_report_ind(BTS2S_AV_INST_DATA *clt_data, U16 conn_id);

void avs_av_conn_cfm(U16 apphdl,
                     U8 res,
                     U16 conn_id,
                     BTS2S_BD_ADDR bd,
                     U16 mtu_size);
void avs_av_conn_ind(U16 apphdl, U16 conn_id, BTS2S_BD_ADDR bd);


void avs_av_enb_cfm(U16 apphdl, U16 enable_role, U8 res);
void avs_av_disb_cfm(U16 apphdl, U8 res);

#ifdef __cplusplus
}
#endif

#endif

