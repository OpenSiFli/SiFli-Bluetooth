/*
 * SPDX-FileCopyrightText: 2024-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BT_DIS_API_H_
#define _BT_DIS_API_H_

#ifdef __cplusplus
extern "C" {
#endif

// customize
#if !defined(DIS_VENDOR_ID)
#define DIS_VENDOR_ID  0x0AC4   //SiFli Technologies (shanghai) Inc
#endif
#if !defined(DIS_PRODUCT_ID)
#define DIS_PRODUCT_ID 0x0000
#endif
#if !defined(DIS_PRODUCT_VERSION)
#define DIS_PRODUCT_VERSION 0x001F  //0.1.F
#endif

// values should keep untouched
#define DIS_VENDID_SOURCE_BTSIG        0x0001
#define DIS_VENDID_SOURCE_USBIF        0x0002


typedef enum
{
    BTS2MD_BT_DIS_SDP_REG_REQ = BTS2MD_START,//0x00
    BTS2MD_BT_DIS_SDP_UNREG_REQ,
} bt_dis_event_t;

typedef enum
{
    BTS2MU_BT_DIS_SDP_RES = BTS2MU_START,
    BTS2MU_BT_DIS_SDP_UNRES,
} bt_dis_response_t;

typedef struct
{
    uint8_t  prim_rec;
    uint16_t spec_id;
    uint16_t prod_id;
    uint16_t prod_ver;
    uint16_t vend_id;
    uint16_t vend_id_source;
} bt_dis_device_info_t;

typedef struct
{
    U16 type;
    bt_dis_device_info_t dis_info;
} bt_dis_sdp_reg_req_t;

typedef struct
{
    U16 type;
    uint32_t sdp_sec_hdl;
} bt_dis_sdp_unreg_req_t;

typedef struct
{
    U16 type;
    uint8_t res;
    uint32_t sdp_sec_hdl;
} bt_dis_sdp_reg_cfm_t;

void bt_dis_sdp_req_api(bt_dis_device_info_t *dis_info);
void bt_dis_sdp_unreq_api(U32 sdp_rec_hdl);
void bt_dis_sdp_status_cfm(U16 type, U32 sdp_sec_hdl, U8 res);

#ifdef __cplusplus
}
#endif
#endif

