/*
 * SPDX-FileCopyrightText: 2021-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __AMS_SERVICE_H
#define __AMS_SERVICE_H

#include "bf0_ble_ams.h"

#ifndef BSP_USING_PC_SIMULATOR
    #define __ARRAY_EMPTY_
#endif

#define AMS_SERVICE_DEFAULT_PLAYER_MASK (BLE_AMS_PLAYER_ATTR_ID_ALL_MASK)
#define AMS_SERVICE_DEFAULT_QUEUE_MASK (BLE_AMS_QUEUE_ATTR_ID_ALL_MASK)
#define AMS_SERVICE_DEFAULT_TRACK_MASK (BLE_AMS_TRACK_ATTR_ID_ALL_MASK)



typedef enum
{
    AMS_SERVICE_SET_PLAYER_ATTRIBUTE_MASK,
    AMS_SERVICE_SET_QUEUE_ATTRIBUTE_MASK,
    AMS_SERVICE_SET_TRACK_ATTRIBUTE_MASK,
    AMS_SERVICE_SEND_REMOTE_COMMAND,
    AMS_SERVICE_ENABLE_CCCD,
} ams_service_command_t;



typedef struct
{
    ams_service_command_t command;
    union
    {
        uint8_t player_mask;
        uint8_t queue_mask;
        uint8_t track_mask;
        ble_ams_cmd_t remote_cmd;
        uint8_t enable_cccd;
    } data;
} ams_service_config_t;

#endif // __AMS_SERVICE_H




