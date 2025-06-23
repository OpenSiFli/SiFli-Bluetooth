/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_SIBLES_DFU_H
#define __BF0_SIBLES_DFU_H

typedef struct
{
    uint8_t img_id;
    uint8_t img_size;
} ble_dfu_start_request_t;

typedef struct
{
    uint8_t result;
} ble_dfu_start_request_response_t;

typedef struct
{
    uint8_t percent;
} ble_dfu_packet_recv_t;

typedef struct
{
    uint8_t result;
} ble_dfu_end_t;

typedef enum
{
    BLE_DFU_EVENT_SUCCESSED,
    BLE_DFU_EVENT_POSTPONE,
    BLE_DFU_EVENT_FAILED,
} ble_dfu_event_ack_t;



typedef enum
{
    BLE_DFU_START_REQUEST,
    BLE_DFU_PACKET_RECV,
    BLE_DFU_END,
} ble_dfu_event_t;

typedef uint8_t (*ble_dfu_callback)(uint16_t event, void *param);

void ble_dfu_register(ble_dfu_callback callback);

void ble_dfu_respond_start_request(ble_dfu_start_request_response_t *rsp);





#endif // __BF0_SIBLES_DFU_H



