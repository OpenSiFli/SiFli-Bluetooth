/**
  ******************************************************************************
  * @file   bf0_sibles_serial_trans_client.h
  * @author Sifli software development team
  * @brief Header file - Sibles serial transfer service.
 *
  ******************************************************************************
*/
/*
 * @attention
 * Copyright (c) 2025 - 2025,  Sifli Technology
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Sifli integrated circuit
 *    in a product or a software update for such product, must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Sifli nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Sifli integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY SIFLI TECHNOLOGY "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SIFLI TECHNOLOGY OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
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

/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/
