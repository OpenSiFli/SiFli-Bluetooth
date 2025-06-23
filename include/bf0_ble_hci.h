/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BF0_BLE_HCI_H
#define __BF0_BLE_HCI_H


/*
 * INCLUDE FILES
 ****************************************************************************************
    */
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "bf0_sibles_internal.h"

/*
 * DEFINES
 ****************************************************************************************
 */


enum ble_hci_event_t
{
    BLE_HCI_SEND_COMMAND_CNF = BLE_HCI_TYPE,
    BLE_HCI_EVENT_IND,
    BLE_HCI_SEND_DATA_CNF,
    BLE_HCI_DATA_IND
};


typedef struct
{
    uint8_t event_id;
    uint8_t len;
    uint8_t *data;
} ble_hci_evt_t;

typedef struct
{
    uint16_t opcode;
    uint8_t len;
    uint8_t data[__ARRAY_EMPTY];
} ble_hci_command_t;




typedef struct
{
    ///Connection handle & Data Flags
    uint16_t handle: 12;
    uint16_t pb_flag: 2;
    uint16_t bc_flag: 2;
    ///Data length in number of bytes
    uint16_t len;
    uint8_t data[__ARRAY_EMPTY];
} ble_hci_data_t;



uint8_t ble_hci_send_command(ble_hci_command_t *command);

uint8_t ble_hci_send_data(ble_hci_data_t *data);

void sifli_hci_process(void);


#endif // __BF0_BLE_HCI_H


