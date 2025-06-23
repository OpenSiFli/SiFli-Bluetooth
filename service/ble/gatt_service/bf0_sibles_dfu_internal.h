/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_SIBLES_DFU_INTERNAL_H
#define __BF0_SIBLES_DFU_INTERNAL_H

#include "dfu.h"

#define BLE_DFU_PACKET_SIZE 128

#define BLE_DFU_HEADER_LEN 348  // 32 + 316
#define BLE_DFU_BODY_LEN 548 // 32 + 4+ 512
#define BLE_DFU_HCPU_FLAG 146

#ifndef __ARRAY_EMPTY
    #define __ARRAY_EMPTY
#endif

typedef enum
{
    BLE_DFU_STATE_IDLE,
    BLE_DFU_STATE_NEGOTIATING,
    BLE_DFU_STATE_READY,
    BLE_DFU_STATE_RECEVING,
} ble_dfu_state_t;

typedef enum
{
    BLE_DFU_TYPE_VER_INQUIRY,
    BLE_DFU_TYPE_VER_INQUIRY_RSP,
    BLE_DFU_TYPE_NEG_REQ,
    BLE_DFU_TYPE_NEG_RSP,
    BLE_DFU_TYPE_RESUME_REQ,
    BLE_DFU_TYPE_RESUME_RSP,
    BLE_DFU_TYPE_PACKET_SEND_REQ,
    BLE_DFU_TYPE_PACKET_SEND_RSP,
    BLE_DFU_TYPE_PACKET,
    BLE_DFU_TYPE_PACKET_CFM,
    BLE_DFU_TYPE_PACKET_SEND_END
} ble_dfu_type_t;

typedef struct
{
    uint32_t total_len;
    uint32_t curr_len;
    uint16_t curr_count;
    uint16_t packet_size;
    uint8_t img_id;
} ble_dfu_info_t;

typedef struct
{
    uint32_t processed_len;
    uint8_t *dfu_data;
} ble_dfu_data_t;

typedef __PACKED_STRUCT
{
    // reserved first
    uint8_t img_count;
    uint8_t img_id[__ARRAY_EMPTY];
} ble_dfu_ver_inquiry_t;

typedef __PACKED_STRUCT
{
    uint8_t img_id;
    uint8_t img_state;
    uint8_t img_curr_ver[DFU_VERSION_LEN];
    uint8_t img_target_ver[DFU_VERSION_LEN];
    uint32_t img_target_len;
} ble_dfu_ver_info_t;

typedef __PACKED_STRUCT
{
    uint8_t msg_type;
    uint8_t img_count;
    ble_dfu_ver_info_t info[__ARRAY_EMPTY];
} ble_dfu_ver_inquiry_rsp_t;

typedef __PACKED_STRUCT
{
    uint16_t packet_size;
} ble_dfu_neg_req_t;

typedef __PACKED_STRUCT
{
    uint8_t type;
    uint8_t result;
    uint16_t packet_size;
} ble_dfu_neg_rsp_t;

#ifdef BSP_USING_DFU_COMPRESS
typedef __PACKED_STRUCT
{
    uint8_t flashid;
    struct image_header_compress_resume data;
} ble_dfu_resume_req_t;
#endif

typedef __PACKED_STRUCT
{
    uint8_t result;
} ble_dfu_resume_rsp_t;

typedef __PACKED_STRUCT
{
    uint32_t total_len;
    uint8_t img_id;
} ble_dfu_packet_send_req_t;

typedef __PACKED_STRUCT
{
    uint8_t type;
    uint8_t result;
} ble_dfu_packet_send_rsp_t;

typedef __PACKED_STRUCT
{
    uint16_t count;
    uint16_t size;
    uint8_t packet[__ARRAY_EMPTY];
} ble_dfu_packet_t;

typedef __PACKED_STRUCT
{
    uint8_t type;
    uint8_t result;
} ble_dfu_packet_cfm_t;


typedef struct
{
    ble_dfu_state_t state;
    ble_dfu_info_t info;
    ble_dfu_callback callback;
    uint8_t conn_idx;
    ble_dfu_data_t data;
} ble_dfu_env_t;

#endif // __BF0_SIBLES_DFU_INTERNAL_H



