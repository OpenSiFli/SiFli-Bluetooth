/*
 * SPDX-FileCopyrightText: 2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef BSP_BLE_SERIAL_TRANSMISSION

#ifndef _BF0_SIBLES_SERIAL_TRANS_CLIENT_H
#define _BF0_SIBLES_SERIAL_TRANS_CLIENT_H

#include "bf0_sibles_serial_trans_service.h"

/**
 * @defgroup Serial_tran Serial transmission client
 * @ingroup profile
 * @{
 */

#if defined(_MSC_VER)
#pragma section("SerialTranClientExport$f",read)
/**
 * @brief The macro that register user categoryID and callback.
 * @param[in] cate_id categoryID.
 * @param[in] callback user callback.
 */
#define BLE_SERIAL_CLIENT_EXPORT(cate_id, callback)                                             \
__declspec(allocate("SerialTranClientExport$f"))const ble_serial_tran_export_t _serial_client_export= \
    {                                                                          \
        callback,                                                              \
        cate_id,                                                                   \
    }
#pragma comment(linker, "/merge:RSerialTranClientExport=mytext")
#else
/**
 * @brief The macro that register user categoryID and callback.
 * @param[in] cate_id categoryID.
 * @param[in] callback user callback.
 */
#define BLE_SERIAL_CLIENT_EXPORT(cate_id, callback)                \
RT_USED static const ble_serial_tran_export_t _serial_client_export   \
SECTION("SerialTranClientExport") =                                              \
{                                                                          \
    callback,                                                              \
    cate_id,                                                                   \
}
#endif

/**
 * @brief Enable serial transmission client.
 */
void ble_serial_client_init(void);

/**
* @brief Open serial transmission client.
* @param[in] conn_idx .
* @retval result BLE_SERIAL_TRAN_ERROR_NO_ERROR is successful, others are failed.
*/
ble_serial_tran_error_event_t ble_serial_client_open(uint8_t conn_idx);

/**
 * @brief Send serial transmission client data.
 * @param[in] data serial transmission data.
 * @param[in] is_retry if retry send data when gatt queue is full.
 * @retval result BLE_SERIAL_TRAN_ERROR_NO_ERROR is successful, others are failed.
 */
ble_serial_tran_error_event_t ble_serial_client_send_data(ble_serial_tran_data_t *data, uint8_t is_retry);


/**
* @}
*/



#endif // _BF0_SIBLES_SERIAL_TRANS_CLIENT_H
#endif //BSP_BLE_SERIAL_TRANSMISSION


