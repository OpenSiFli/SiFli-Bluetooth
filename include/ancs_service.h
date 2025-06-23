/*
 * SPDX-FileCopyrightText: 2021-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ANCS_SERVICE_H
#define __ANCS_SERVICE_H

#include "bf0_ble_ancs.h"


#define __ARRAY_EMPTY_


#define ANCS_SERVICE_DEFAULT_ATTR_MASK (BLE_ANCS_NOTIFICATION_ATTR_ID_MASK_APP_ID | \
                                        BLE_ANCS_NOTIFICATION_ATTR_ID_MASK_TITLE  | \
                                        BLE_ANCS_NOTIFICATION_ATTR_ID_MASK_MESSAGE)

#define ANCS_SERVICE_DEFAULT_CATE_MASK (BLE_ANCS_CATEGORY_ID_MASK_ALL)

#define ANCS_SERVICE_TITLE_LEN (100)
#define ANCS_SERVICE_SUBTITLE_LEN (100)
#define ANCS_SERVICE_MESSAGE_LEN (1024)
#define ANCS_SERVICE_MAX_APP (10)


#define ANCS_APP_ID_LEN (35)
#define ANCS_APP_NAME_LEN (20)



typedef enum
{
    ANCS_SERVICE_SET_ATTRIBUTE_MASK,
    ANCS_SERVICE_SET_CATEGORY_MASK,
    ANCS_SERVICE_ENABLE_CCCD,
    ANCS_SERVICE_PERFORM_NOTIFY_ACTION
} ancs_service_command_t;



typedef struct
{
    ancs_service_command_t command;
    union
    {
        uint16_t attr_mask;
        uint16_t cate_mask;
        uint8_t enable_cccd;
        struct
        {
            uint32_t uid;
            ble_ancs_action_id_val_t act_id;
        } action;
    } data;
} ancs_service_config_t;


typedef struct
{
    uint8_t evt_id;                 /**< Event ID(@see enum ble_ancs_event_id_t) */
    uint8_t evt_flag;               /**< Event flag(@see enum ble_ancs_event_flag_t). */
    uint8_t cate_id;                /**< Category ID(@see enum ble_ancs_category_id_t). */
    uint8_t cate_count;             /**< Category count. */
    uint32_t noti_uid;              /**< Noti UID. */
    uint8_t attr_count;             /**< Notification attribute count. */
    ble_ancs_attr_value_t value[__ARRAY_EMPTY_];   /**< Notification attribute value. */
} ancs_service_noti_attr_t;



#endif // __ANCS_SERVICE_H




