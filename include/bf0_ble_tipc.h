/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_BLE_TIPC_H
#define __BF0_BLE_TIPC_H

#include "bf0_ble_common.h"


enum ble_tipc_event
{
    BLE_TIPC_CURRENT_TIME_NOTIFY = BLE_TIP_TYPE,
    BLE_TIPC_READ_CURRENT_TIME_RSP,
    BLE_TIPC_READ_LOCAL_INFO_RSP,
};

/// Time profile information
typedef struct
{
    /// year time element
    uint16_t year;
    /// month time element
    uint8_t month;
    /// day time element
    uint8_t day;
    /// hour time element
    uint8_t hour;
    /// minute time element
    uint8_t min;
    /// second time element
    uint8_t sec;
} ble_tipc_date_time_t;


///Current Time Characteristic Structure
typedef struct
{
    /// Date time
    ble_tipc_date_time_t date_time;
    /// Day of the week
    uint8_t day_of_week;
    /// 1/256th of a second
    uint8_t fraction_256;
    /// Adjust reason. Bit 0: Manual time update. Bit 1: External reference time update. Bit 2: Change of timezone.
    /// Bit 3: Change of DST.
    uint8_t adjust_reason;
} ble_tipc_curr_time_t;

///Local Time Info Characteristic Structure - UUID:0x2A0F
typedef struct
{
    /// 15 minutes increments between local time and UTC. Valid range is -48-56 and -128 means unknown.
    uint8_t time_zone;
    /// DaySaving Time. 0: Standard time. 2: +0.5h. 4: +1h. 8: +2h. 255: unknown. Others are reserved.
    uint8_t dst_offset;
} ble_tip_local_time_info_t;


void ble_tipc_init(uint8_t enable);

int8_t ble_tipc_read_current_time(uint8_t conn_idx);

int8_t ble_tipc_read_local_time_info(uint8_t conn_idx);

int8_t ble_tipc_enable(uint8_t conn_idx);

#endif //__BF0_BLE_TIPC_H


