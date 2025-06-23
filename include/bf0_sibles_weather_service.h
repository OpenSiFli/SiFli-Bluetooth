/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_SIBLES_WEATHER_SERVICE_h
#define __BF0_SIBLES_WEATHER_SERVICE_h


typedef struct
{
    uint8_t city_id[9];
    int8_t high_temp;
    int8_t low_temp;
    uint8_t type;
    uint8_t day;
    uint8_t week;
} ble_weather_srv_data_t;

typedef struct
{
    uint8_t city_id[9];
    uint8_t city_name[10];
} ble_weather_srv_city_id_name_t;


typedef enum
{
    BLE_WEATHER_SRV_TYPE_SUN,
    BLE_WEATHER_SRV_TYPE_RAIN,
    BLE_WEATHER_SRV_TYPE_CLOUD,
    BLE_WEATHER_SRV_TYPE_FOG,
} ble_weather_srv_type_t;

typedef struct
{
    uint8_t type_name[8];
} ble_weather_type_name_t;



typedef enum
{
    BLE_WEATHER_SRV_WEEK_MON,
    BLE_WEATHER_SRV_WEEK_TUE,
    BLE_WEATHER_SRV_WEEK_WEN,
    BLE_WEATHER_SRV_WEEK_THUR,
    BLE_WEATHER_SRV_WEEK_FRI,
    BLE_WEATHER_SRV_WEEK_SAT,
    BLE_WEATHER_SRV_WEEK_SUN,
} ble_weather_srv_week_t;

typedef struct
{
    uint8_t week_name[5];
} ble_weather_week_name_t;


const uint8_t *ble_weather_srv_get_city_name(uint8_t *city_id);

int ble_weather_srv_register(void);

#endif


