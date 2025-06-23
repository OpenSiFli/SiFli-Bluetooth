/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BT_GATT_API_H_
#define _BT_GATT_API_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "bts2_bt.h"
#include "bts2_msg.h"

typedef enum
{
    BTS2MD_BT_GATT_SDP_REG_REQ = BTS2MD_START,//0x00
    BTS2MD_BT_GATT_SDP_UNREG_REQ,
    BTS2MD_BT_GATT_CONN_REQ,
    BTS2MD_BT_GATT_DISC_REQ,
    BTS2MD_BT_GATT_SEND_DATA_REQ,
    BTS2MD_BT_GATT_CHANGE_MTU_REQ,
    BTS2MD_BT_GATT_REQ_MAX
} BT_GATT_EVENT_REQ_T;

enum
{
    BTS2MU_BT_GATT_SDP_RES = BTS2MU_START,
    BTS2MU_BT_GATT_SDP_UNRES,
    BTS2MU_BT_GATT_MTU_RES,
    BTS2MD_BT_GATT_CONN_RES,
    BTS2MD_BT_GATT_DISC_RES,
    BTS2MU_BT_GATT_CONN_STATE,
    BTS2MD_BT_GATT_RES_MAX
};

typedef enum
{
    BT_GATT_DISCONNECTED = 0,
    BT_GATT_CONNECTING,
    BT_GATT_CONNECTED,
    BT_GATT_DISCONNECTING,
} bt_gatt_device_state_t;

typedef struct
{
    U16          type;
    BTS2S_BD_ADDR bd;
} BTS2S_BT_GATT_CONN_INFO;

typedef struct
{
    U16 type;
    U16 payload_len;
    char *payload;
} BTS2S_BT_GATT_DATA;

typedef struct
{
    U16 type;
    U16 mtu;
} BTS2S_BT_GATT_MTU;

typedef struct
{
    U16 type;
    BTS2S_BD_ADDR bd;
    U8 device_state;
    U8 res;
} BTS2S_BT_GATT_RES;

typedef struct
{
    U16 type;
    U32 sdp_rec_hdl;
    U8 res;
} bt_gatt_sdp_reg_info;

typedef struct
{
    U16 type;
    U16 mtu;
    U8 res;
} bt_gatt_mtu_res_info;

typedef struct
{
    U16 gatt_start_handle;
    U16 gatt_end_handle;
    U8 att_uuid_len;
    U8 *att_uuid;
} br_att_sdp_data_t;

void bt_gatt_conn_req(BTS2S_BD_ADDR *bd);
void bt_gatt_disconn_req(BTS2S_BD_ADDR *bd);
void bt_gatt_send_data_req(char *payload, U16 payload_len);
void bt_gatt_change_mtu_req(U16 mtu);
void bt_gatt_create_sdp_record(U8 att_uuid_len, U8 *att_uuid, U16 gatt_start_handle, U16 gatt_end_handle);
void bt_gatt_create_sdp_unreg(U32 svc_rec_hdl);

#ifdef __cplusplus
}
#endif
#endif

