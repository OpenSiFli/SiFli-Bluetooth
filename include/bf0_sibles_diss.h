/*
 * SPDX-FileCopyrightText: 2024-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_BLE_DIS_H
#define __BF0_BLE_DIS_H

/*
 * DEFINES
 ****************************************************************************************
 */

/// Manufacturer Name Value
#define SIBLES_DIS_MANUFACTURER_NAME       ("Sifli")
#define SIBLES_DIS_MANUFACTURER_NAME_LEN   (5)

/// Model Number String Value
#define SIBLES_DIS_MODEL_NB_STR            ("SF-BLE-1.0")
#define SIBLES_DIS_MODEL_NB_STR_LEN        (10)

/// Serial Number
#define SIBLES_DIS_SERIAL_NB_STR           ("1.0.0.0-LE")
#define SIBLES_DIS_SERIAL_NB_STR_LEN       (10)

/// Firmware Revision
#define SIBLES_DIS_FIRM_REV_STR            ("6.1.2")
#define SIBLES_DIS_FIRM_REV_STR_LEN        (5)

/// System ID Value - LSB -> MSB
#define SIBLES_DIS_SYSTEM_ID               ("\x12\x34\x56\xFF\xFE\x9A\xBC\xDE")
#define SIBLES_DIS_SYSTEM_ID_LEN           (8)

/// Hardware Revision String
#define SIBLES_DIS_HARD_REV_STR           ("1.0.0")
#define SIBLES_DIS_HARD_REV_STR_LEN       (5)

/// Software Revision String
#define SIBLES_DIS_SW_REV_STR              ("6.3.0")
#define SIBLES_DIS_SW_REV_STR_LEN          (5)

/// IEEE
#define SIBLES_DIS_IEEE                    ("\xFF\xEE\xDD\xCC\xBB\xAA")
#define SIBLES_DIS_IEEE_LEN                (6)

/**
 * PNP ID Value - LSB -> MSB
 *      Vendor ID Source : 0x02 (USB Implementer’s Forum assigned Vendor ID value)
 *      Vendor ID : 0x045E      (Microsoft Corp)
 *      Product ID : 0x0040
 *      Product Version : 0x0300
 */
#define SIBLES_DIS_PNP_ID               ("\x02\x5E\x04\x40\x00\x00\x03")
#define SIBLES_DIS_PNP_ID_LEN           (7)

/// Maximal length for Characteristic values - 128 bytes
#define SIBLES_DIS_VAL_MAX_LEN                         (128)
///IEEE Certif length (min 6 bytes)
#define SIBLES_DIS_IEEE_CERTIF_MIN_LEN                 (0x06)

#define SIBLES_DIS_SYS_ID_LEN                          (0x08)

enum bf0_dis_event_t
{
    BLE_DIS_GET_MANU_NAME,
    BLE_DIS_GET_MODEL_NB,
    BLE_DIS_GET_SERI_NB,
    BLE_DIS_GET_HW_REV,
    BLE_DIS_GET_FW_REV,
    BLE_DIS_GET_SW_REV,
    BLE_DIS_GET_SYS_ID,
    BLE_DIS_GET_IEEE_DATA,
    BLE_DIS_GET_PNP_ID,
};

/**
  * @brief  callback for read response.
  * @param[in]  conn_idx readd connection index.
  * @param[in]  read event see@bf0_dis_event_t.
  * @param[in]  read response length.
  * @return[out]  read response content.
  */
typedef uint8_t *(*ble_dis_callback)(uint8_t conn_idx, uint8_t event, uint16_t *len);

/**
  * @brief  register Bluetooth Device information service server.
  * @param[in]  callback read rsp defined by caller.
  */
void sibles_ble_diss_init(ble_dis_callback callback);

#endif //__BF0_BLE_DIS_H


