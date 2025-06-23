/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef BSP_BLE_LOG

#ifndef _BF0_SIBLES_LOG_H
#define _BF0_SIBLES_LOG_H

#define MAX_PACKCT_SIZE 243
#define SERIAL_TRANS_HEADER 4
#define BLE_LOG_SEND_HEADER 5

#define BLE_LOG_VERSION 1

enum ble_log_command
{
    BLE_LOG_COMMAND_DISABLE,
    BLE_LOG_COMMAND_ENABLE,
    BLE_LOG_COMMAND_GET,
};

enum ble_log_status
{
    BLE_LOG_STATUS_OK,
    BLE_LOG_STATUS_DISABLE,
};

enum ble_log_state
{
    BLE_LOG_STATE_NONE,
    BLE_LOG_STATE_TRANSPORT,
};

typedef struct
{
    uint32_t start_addr;
    uint32_t total_size;
    uint32_t receive_size;
    uint32_t receive_index;
} ble_phone_send_data;

typedef struct
{
    uint8_t is_open;
    /* Handle for serial tran service. */
    uint8_t handle;

    uint8_t conn_idx;
    uint8_t conn_state;

    uint16_t mtu;

    // totol log data size
    uint32_t size;
    uint8_t *addr;

    uint32_t send_index;
    uint8_t command;
    uint8_t state;

    ble_phone_send_data receive_data;
} ble_log_env_t;

typedef enum
{
    BLE_LOG_CLEAR,
    BLE_LOG_GET,
    BLE_LOG_GET_SEND_START,
    BLE_LOG_GET_SEND_PACKET,
    BLE_LOG_GET_SEND_END,
    BLE_LOG_STATUS_INQUIRY,
    BLE_LOG_STATUS_REPLY,
    BLE_LOG_GET_SEND_PACKET_RSP,
    BLE_LOG_PHONE_DATA_SEND_START_REQ,
    BLE_LOG_PHONE_DATA_SEND_START_RSP,
    BLE_LOG_PHONE_DATA_SEND,
    BLE_LOG_PHONE_DATA_SEND_END,
    BLE_LOG_TSDB_GET,
    BLE_LOG_TSDB_METRICS_GET,
    BLE_LOG_TSDB_SWITCH_RESULT,

    BLE_LOG_GET_SEND_FINISH = 15,
    BLE_LOG_ON_OFF = 16,

    BLE_LOG_ASSERT_GET = 17,
    BLE_LOG_ASSERT_CLEAR,
    BLE_LOG_ASSERT_ON_OFF,

    BLE_LOG_METRICS_GET = 20,
    BLE_LOG_METRICS_CLEAR,
    BLE_LOG_METRICS_ON_OFF,

    BLE_LOG_MONKEY_GET = 23,
    BLE_LOG_MONKEY_CLEAR,

    BLE_LOG_OP_FINISH = 25,

    BLE_AUDIO_DUMP_ENABLE = 30,
    BLE_AUDIO_DUMP_GET_RSP = 31,
    BLE_AUDIO_DUMP_CLEAR = 32,
    BLE_AUDIO_DUMP_DELAY = 33,
    BLE_AUDIO_DUMP_COMMAND = 34,
    BLE_AUDIO_DUMP_GET = 35,
    BLE_MEM_GET_REQ,
    BLE_MEM_GET_RSP,
    BLE_MEM_SET,

    BLE_HCI_LOG_ON_OFF = 39,
    BLE_HCI_LOG_CLEAR = 40,
    BLE_HCI_LOG_GET = 41,

    BLE_DEV_INFO_GET = 42,
    BLE_DEV_REMOTE_INFO_GET_RSP = 43,

    BLE_TX_POWER_SET = 44,

    BLE_ASSERT_CMD = 45,
    BLE_FINSH_CMD = 46,

    BLE_LOG_INQUIRY_REQ = 47,
    BLE_LOG_INQUIRY_RSP = 48,
} ble_log_protocol_msg_id_t;

typedef enum
{
    BLE_AUDIO_DUMP_DOWLINK,
    BLE_AUDIO_DUMP_DOWLINK_AGC,
    BLE_AUDIO_DUMP_ADUPRC,
    BLE_AUDIO_DUMP_DC_OUTPUT,
    BLE_AUDIO_DUMP_RAMP_IN_OUTPUT,
    BLE_AUDIO_DUMP_AECM_INPUT1,
    BLE_AUDIO_DUMP_AECM_INPUT2,
    BLE_AUDIO_DUMP_AECM_OUTPUT,
    BLE_AUDIO_DUMP_ANC_OUTPUT,
    BLE_AUDIO_DUMP_AGC_OUTPUT,
    BLE_AUDIO_DUMP_RAMP_OUT_OUTPUT,
} ble_audio_dump_type;

typedef struct
{
    uint16_t message_id; /* ref @ble_log_protocol_msg_id_t */
    uint16_t length;
    uint8_t data[0];
} ble_log_protocol_t;

/**
 * get ble log state
 * return: BLE_LOG_STATE_TRANSPORT if is transporting log to phone
 * BLE_LOG_STATE_NONE for no log work for now
 */
uint8_t ble_log_get_transport_state();

#endif // _BF0_SIBLES_LOG_H

#endif // BSP_BLE_LOG



