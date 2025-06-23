/*
 * SPDX-FileCopyrightText: 2022-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BT_RT_DEVICE_URC_HF_H
#define _BT_RT_DEVICE_URC_HF_H

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "bts2_bt.h"


#ifdef BT_USING_SIRI
    extern    void urc_func_bt_voice_recog_sifli(uint8_t status);
    extern    void urc_func_bt_hf_voice_recog_sifli(uint8_t res);
    extern    void urc_func_bt_voice_recog_cap_sifli(uint8_t status);
#endif

extern    void urc_func_local_phone_number_sifli(char *data, int size);
extern    void urc_func_bt_voice_volume_sifli(uint8_t volume);
extern    void urc_func_bt_dial_complete_sifli(uint8_t res);
extern    void urc_func_bt_cind_sifli(bt_cind_ind_t *ind);
extern    void urc_func_profile_cind_sifli(bts2_hfp_hf_cind *cind);
extern    void urc_func_clcc_sifli(bt_clcc_ind_t *ind);
extern    void urc_func_bt_clcc_complete_sifli(uint8_t res);
extern    void urc_func_bt_call_vol_ind_sifli(uint8_t res);
extern    void urc_func_bt_call_dtmf_ind_sifli(uint8_t res);
extern    void urc_func_hfp_at_cfm_status_notify(uint8_t cmd_id, uint8_t res);
extern    int bt_sifli_notify_hfp_hf_event_hdl(uint16_t event_id, uint8_t *data, uint16_t data_len);
#endif /* _BT_RT_DEVICE_URC_H */



