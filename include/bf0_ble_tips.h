/*
 * SPDX-FileCopyrightText: 2024-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_BLE_TIPS_H
#define __BF0_BLE_TIPS_H


enum bf0_tips_event_t
{
    BLE_TIPS_GET_CURRENT_TIME,
    BLE_TIPS_GET_LOCAL_TIME_INFO,
    BLE_TIPS_GET_REF_TIME_INFO,
    BLE_TIPS_SET_CURRENT_TIME,
    BLE_TIPS_SET_LOCAL_TIME_INFO,
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
} ble_tips_date_time_t;

///Current Time Characteristic Structure
typedef struct
{
    /// Date time
    ble_tips_date_time_t date_time;
    /// Day of the week
    uint8_t day_of_week;
    /// 1/256th of a second
    uint8_t fraction_256;
    /// Adjust reason. Bit 0: Manual time update. Bit 1: External reference time update. Bit 2: Change of timezone.
    /// Bit 3: Change of DST.
    uint8_t adjust_reason;
} ble_tips_cur_time_t;

///Local Time Info Characteristic Structure - UUID:0x2A0F
typedef struct
{
    /// 15 minutes increments between local time and UTC. Valid range is -48-56 and -128 means unknown.
    uint8_t time_zone;
    /// DaySaving Time. 0: Standard time. 2: +0.5h. 4: +1h. 8: +2h. 255: unknown. Others are reserved.
    uint8_t dst_offset;
} ble_tips_local_time_info_t;

/**
 * Time Source Characteristic - UUID:0x2A13
 * Min value : 0, Max value : 6
 * 0 = Unknown
 * 1 = Network Time Protocol
 * 2 = GPS
 * 3 = Radio Time Signal
 * 4 = Manual
 * 5 = Atomic Clock
 * 6 = Cellular Network
 */
typedef uint8_t tips_time_source;

/**
 * Time Accuracy Characteristic - UUID:0x2A12
 * Accuracy (drift) of time information in steps of 1/8 of a second (125ms) compared
 * to a reference time source. Valid range from 0 to 253 (0s to 31.5s). A value of
 * 254 means Accuracy is out of range (> 31.5s). A value of 255 means Accuracy is
 * unknown.
 */
typedef uint8_t tips_time_accuracy;

///Reference Time Info Characteristic Structure - UUID:0x2A14
typedef struct
{
    tips_time_source time_source;
    tips_time_accuracy time_accuracy;
    /**
     * Days since last update about Reference Source
     * Min value : 0, Max value : 254
     * 255 = 255 or more days
     */
    uint8_t days_update;
    /**
     * Hours since update about Reference Source
     * Min value : 0, Mac value : 23
     * 255 = 255 or more days (If Days Since Update = 255, then Hours Since Update shall
     * also be set to 255)
     */
    uint8_t hours_update;
} ble_tips_ref_time_info_t;

typedef struct
{
    ble_tips_cur_time_t cur_time;
    ble_tips_local_time_info_t local_time_inf;
    ble_tips_ref_time_info_t ref_time_inf;
} ble_tips_time_env_t;


typedef uint8_t *(*ble_tips_callback)(uint8_t conn_idx, uint8_t event, uint8_t *value);

/**
 * @brief initiate time profile server
   @param[in] an initialization time env.
 */
void ble_tips_init(ble_tips_callback callback, ble_tips_time_env_t time_env);

/**
 * @brief local server notify remote current time has changed.
   @param[in] conn_idx connection index.
   @param[in] current time struct.
   @retval result 0 is successful.
 */
int8_t ble_tips_notify_current_time(uint8_t conn_idx, ble_tips_cur_time_t cur_time);


#endif //__BF0_BLE_TIPS_H


