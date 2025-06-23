/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BF0_SIBLES_ADVERTISING_INTERNAL_H_
#define BF0_SIBLES_ADVERTISING_INTERNAL_H_

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "bf0_sibles_internal.h"
#include "bf0_sibles_advertising.h"

enum sibles_adv_state_t
{
    SIBLES_ADV_STATE_IDLE,
    SIBLES_ADV_STATE_READY,
    SIBLES_ADV_STATE_STARTING,
    SIBLES_ADV_STATE_STARTED,
    SIBLES_ADV_STATE_STOPPING,
};

#define SIBLES_ADV_FAST_DURATION                    30
#define SIBLES_ADV_BACKGROUD_DURATION               180
#define SIBLES_MAX_ADV_DATA_LENGTH                  251
#define SIBLES_ADV_FLAG_FILED_LEN                   1
#define SIBLES_ADV_LEN_FILED_LEN                    1
#define SIBLES_ADV_TYPE_HEADER_LEN                  (SIBLES_ADV_FLAG_FILED_LEN + SIBLES_ADV_LEN_FILED_LEN)





uint8_t sibles_advertising_data_flags_compose(uint8_t *adv_data, uint8_t *offset,
        uint8_t flags, uint8_t max_len);

uint8_t sibles_advertising_data_tx_pwr_compose(uint8_t *adv_data, uint8_t *offset,
        uint8_t tw_pwr_level, uint8_t max_len);

uint8_t sibles_advertising_data_apperance_compose(uint8_t *adv_data, uint8_t *offset,
        uint16_t appearance, uint8_t max_len);

uint8_t sibles_advertising_data_adv_interval_compose(uint8_t *adv_data, uint8_t *offset,
        uint16_t adv_interval, uint8_t max_len);

uint8_t sibles_advertising_data_local_name_compose(uint8_t *adv_data, uint8_t *offset, sibles_adv_type_name_t *name,
        uint8_t is_shorted, uint8_t max_len);

uint8_t sibles_advertising_data_service_uuid_compose(uint8_t *adv_data, uint8_t *offset, sibles_adv_type_srv_uuid_t *uuid,
        uint8_t is_completed, uint8_t max_len);


uint8_t sibles_advertising_data_service_data_compose(uint8_t *adv_data, uint8_t *offset, sibles_adv_type_srv_data_t *uuid,
        uint8_t max_len);

uint8_t sibles_advertising_data_conn_interval_compose(uint8_t *adv_data, uint8_t *offset, sibles_adv_type_conn_interval_t *conn_interval,
        uint8_t max_len);



uint8_t sibles_advertising_data_manufacturer_compose(uint8_t *adv_data, uint8_t *offset, sibles_adv_type_manufacturer_data_t *manufacturer_data,
        uint8_t max_len);



#endif //BF0_SIBLES_ADVERTISING_INTERNAL_H_


