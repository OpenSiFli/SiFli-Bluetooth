/*
 * SPDX-FileCopyrightText: 2022-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BT_RT_DEVICE_URC_AG_H
#define _BT_RT_DEVICE_URC_AG_H

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "bts2_bt.h"


extern    void urc_func_ag_answer_call_req(void);
extern    void urc_func_ag_hangup_call_req(void);
extern    void urc_func_ag_make_call_req(char *phone_number);
extern    void urc_func_ag_dmtf_key_req(char val);
extern    void urc_func_ag_get_local_phone_info_req(void);
extern    void urc_func_ag_get_indicator_status_req(void);
extern    void urc_func_ag_get_all_call_status_req(void);
extern    void urc_func_bt_voice_volume_sifli(uint8_t volume);
extern    int bt_sifli_notify_hfp_ag_event_hdl(uint16_t event_id, uint8_t *data, uint16_t data_len);

#endif /* _BT_RT_DEVICE_URC_AG_H */



