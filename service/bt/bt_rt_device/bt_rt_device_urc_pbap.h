/*
 * SPDX-FileCopyrightText: 2022-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BT_RT_DEVICE_URC_PBAP_H
#define _BT_RT_DEVICE_URC_PBAP_H

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "bts2_bt.h"


extern    void urc_func_pbap_vcard_list_notify(pbap_vcard_listing_item_t *msg);
extern    void urc_func_pbap_vcard_list_cmp(U8 res);
extern    int bt_sifli_notify_pbap_event_hdl(uint16_t event_id, uint8_t *data, uint16_t data_len);

#endif /* _BT_RT_DEVICE_URC_PBAP_H */



