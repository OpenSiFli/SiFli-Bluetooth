/*
 * SPDX-FileCopyrightText: 2024-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BT_L2CAP_PROFILE_API_H_
#define _BT_L2CAP_PROFILE_API_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BT_L2CAP_PROFILE_DEVICE_DISCONNECTED,
    BT_L2CAP_PROFILE_DEVICE_CONNECTING,
    BT_L2CAP_PROFILE_DEVICE_CONNECTED,
    BT_L2CAP_PROFILE_DEVICE_DISCONNECTING
} BT_L2CAP_PROFILE_DEVICE_ST;

typedef enum
{
    BT_L2CAP_PROFILE_UNREG_STATE,
    BT_L2CAP_PROFILE_REGING_STATE,
    BT_L2CAP_PROFILE_REG_STATE,
    BT_L2CAP_PROFILE_UNREGING_STATE
} BT_L2CAP_PROFILE_REG__STATE;

typedef enum
{
    BTS2MD_BT_L2CAP_PROFILE_REG_REQ = BTS2MD_START,//0x00
    BTS2MD_BT_L2CAP_PROFILE_UNREG_REQ,
    BTS2MD_BT_L2CAP_PROFILE_CONN_REQ,
    BTS2MD_BT_L2CAP_PROFILE_DISC_REQ,
    BTS2MD_BT_L2CAP_PROFILE_SEND_DATA_REQ,
    BTS2MD_BT_L2CAP_PROFILE_CHANGE_MTU_REQ,
    BTS2MD_BT_L2CAP_PROFILE_REQ_MAX
} BT_L2CAP_PROFILE_EVENT_REQ_T;

typedef enum
{
    BTS2MU_BT_L2CAP_PROFILE_REG_CFM = BTS2MU_START,
    BTS2MU_BT_L2CAP_PROFILE_UNREG_CFM,
    BTS2MU_BT_L2CAP_PROFILE_CONN_IND,
    BTS2MU_BT_L2CAP_PROFILE_CONN_CFM,
    BTS2MU_BT_L2CAP_PROFILE_DISC_IND,
    BTS2MU_BT_L2CAP_PROFILE_DISC_CFM,
    BTS2MU_BT_L2CAP_PROFILE_DATA_IND,
    BTS2MU_BT_L2CAP_PROFILE_DATA_CFM,
    BTS2MU_BT_L2CAP_PROFILE_MTU_RES,
    BTS2MU_BT_L2CAP_PROFILE_RES_MAX
} BT_L2CAP_PROFILE_EVENT_RES_T;

typedef struct
{
    U16 type;
    U16 local_psm;
    U16 remote_psm;
    BTS2S_BD_ADDR bd;
} BTS2S_BT_L2CAP_PROFILE_CONN_INFO;

typedef struct
{
    U16 type;
    U16 cid;
    U16 payload_len;
    char *payload;
} BTS2S_L2CAP_PROFILE_DATA;

typedef struct
{
    U16 type;
    U16 hdl_id;
    U16 psm;
    U16 flag;
    U16 flush_timeout;
} BTS2S_L2CAP_PROFILE_REG_INFO;

typedef struct
{
    U16 type;
    U16 hdl_id;
    U16 psm;
} BTS2S_L2CAP_PROFILE_UNREG_INFO;

typedef struct
{
    U16 type;
    U16 flow_mode;
    U16 local_mtu_in;
    U16 local_mtu_out;
    U16 local_flush_in;
    U16 local_flush_out;
} BTS2S_L2CAP_PROFILE_CONFIG_PARA;

typedef struct
{
    U16 type;
    U16 local_psm;
    U16 reg_state;
    U16 res;
} BTS2S_L2CAP_PROFILE_REG_RES;

typedef struct
{
    U16 type;
    U16 cid;
    U16 local_psm;
    U16 remote_mtu;
    U8 device_state;
    U8 res;
    BTS2S_BD_ADDR bd;
} BTS2S_BT_L2CAP_CONN_RES;

typedef struct
{
    U16          type; /* always L2CA_CONN_IND */
    U8           id;   /* used to identify the connection signal */
    U16          cid;  /* channel id */
    U16          psm;  /* the PSM on the local device */
    BTS2S_BD_ADDR bd;  /* bluetooth addr of remote device */
} BTS2S_BT_L2CAP_CONN_IND;

typedef struct
{
    U16 type;
    U16 mtu;
    U8 res;
} BTS2S_BT_L2CAP_MTU_RES;

typedef struct
{
    U16 type;
    U16 cid;
    U16 len;
    U8 *payload;
} BTS2S_BT_L2CAP_DATA_IND;

typedef struct
{
    U16 type;
    U16 cid;
    U8  res;
} BTS2S_BT_L2CAP_DATA_CFM;

void bt_l2cap_profile_reg_req(U16 hdl, U16 psm, U16 flag, U16 flush_timeout);
void bt_l2cap_profile_unreg_req(U16 hdl, U16 psm);
void bt_l2cap_profile_conn_req(BTS2S_BD_ADDR *bd, U16 local_psm, U16 remote_psm);
void bt_l2cap_profile_send_conn_res(U8 accept, BTS2S_BT_L2CAP_CONN_IND *msg);
void bt_l2cap_profile_disconn_req(BTS2S_BD_ADDR *bd, U16 psm);
void bt_l2cap_profile_send_data_req(U16 cid, char *payload, U16 payload_len);


#ifdef __cplusplus
}
#endif
#endif

