/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BF0_BLE_GATT_H_
#define BF0_BLE_GATT_H_

#include "att.h"

// Define a gatt service data base with 128 bit uuid
#define BLE_GATT_SERVICE_DEFINE_128(srv_name) \
struct attm_desc_128 srv_name[] =

// Define a gatt-service data base with 16 bit uuid
#define BLE_GATT_SERVICE_DEFINE_16(srv_name) \
struct attm_desc srv_name[] =

// Declare a gatt service element in a gatt service data base
#define BLE_GATT_SERVICE_DECLARE(idx, uuid, perm) \
    [idx] = {uuid, perm, 0, 0}

// Declare a gatt characterisitc element in a gatt service data base
#define BLE_GATT_CHAR_DECLARE(idx, uuid, perm) \
    [idx] = {uuid, perm, 0, 0}

// Declare a gatt characterisitc value element in a gatt service data base
#define BLE_GATT_CHAR_VALUE_DECLARE(idx, uuid, perm, value_perm, max_len) \
    [idx] = {uuid, perm, value_perm, max_len}

// Declare a gatt descriptor config element in a gatt service data base
#define BLE_GATT_DESCRIPTOR_DECLARE(idx, uuid, perm, value_perm, max_len) \
    [idx] = {uuid, perm, value_perm, max_len}


// INIT a gatt service variable with 128 bit uuid
#define BLE_GATT_SERVICE_INIT_128(var, srv_db, att_num, svc_perm, srv_uuid) \
    sibles_register_svc_128_t var; \
    var.att_db = (struct attm_desc_128 *)&srv_db; \
    var.num_entry = att_num; \
    var.sec_lvl = svc_perm; \
    var.uuid = srv_uuid;

// INIT a gatt service variable with 16 bit uuid
#define BLE_GATT_SERVICE_INIT_16(var, srv_db, att_num, svc_perm, srv_uuid) \
    sibles_register_svc_t var; \
    var.att_db = (struct sibles_register_svc_t *)&srv_db; \
    var.num_entry = att_num; \
    var.sec_lvl = svc_perm; \
    var.uuid = srv_uuid;


// GATT permission
typedef enum
{
    // All permission mask
    BLE_GATT_PERM_MASK_ALL = 0x0,

    // Read permission for diferent security level
    BLE_GATT_PERM_READ_PERMISSION_NO_AUTH = PERM(RP, NO_AUTH),
    BLE_GATT_PERM_READ_PERMISSION_UNAUTH = PERM(RP, UNAUTH),
    BLE_GATT_PERM_READ_PERMISSION_AUTH = PERM(RP, AUTH),
    BLE_GATT_PERM_READ_PERMISSION_SEC_CON = PERM(RP, SEC_CON),

    // Write permission for diferent security level
    BLE_GATT_PERM_WRITE_PERMISSION_NO_AUTH = PERM(WP, NO_AUTH),
    BLE_GATT_PERM_WRITE_PERMISSION_UNAUTH = PERM(WP, UNAUTH),
    BLE_GATT_PERM_WRITE_PERMISSION_AUTH = PERM(WP, AUTH),
    BLE_GATT_PERM_WRITE_PERMISSION_SEC_CON = PERM(WP, SEC_CON),

    // Notification permission for diferent security level
    BLE_GATT_PERM_NOTIFY_PERMISSION_NO_AUTH = PERM(NP, NO_AUTH),
    BLE_GATT_PERM_NOTIFY_PERMISSION_UNAUTH = PERM(NP, UNAUTH),
    BLE_GATT_PERM_NOTIFY_PERMISSION_AUTH = PERM(NP, AUTH),
    BLE_GATT_PERM_NOTIFY_PERMISSION_SEC_CON = PERM(NP, SEC_CON),

    // Indication permission for diferent security level
    BLE_GATT_PERM_INDICATE_PERMISSION_NO_AUTH = PERM(IP, NO_AUTH),
    BLE_GATT_PERM_INDICATE_PERMISSION_UNAUTH = PERM(IP, UNAUTH),
    BLE_GATT_PERM_INDICATE_PERMISSION_AUTH = PERM(IP, AUTH),
    BLE_GATT_PERM_INDICATE_PERMISSION_SEC_CON = PERM(IP, SEC_CON),

    // Read access enabled
    BLE_GATT_PERM_READ_ENABLE = PERM(RD, ENABLE),

    // Write command access enabled
    BLE_GATT_PERM_WRITE_COMMAND_ENABLE = PERM(WRITE_COMMAND, ENABLE),

    // Write request access enabled
    BLE_GATT_PERM_WRITE_REQ_ENABLE = PERM(WRITE_REQ, ENABLE),

    // Notification access enabled
    BLE_GATT_PERM_NOTIFY_ENABLE = PERM(NTF, ENABLE),

    // Indication access enabled
    BLE_GATT_PERM_INDICATE_ENABLE = PERM(IND, ENABLE),

} ble_gatt_perm_t;


// GATT value permission
typedef enum
{
    BLE_GATT_VALUE_PERM_MASK_ALL = 0,

    // length of gatt uuid
    BLE_GATT_VALUE_PERM_UUID_16 = PERM(UUID_LEN, UUID_16),
    BLE_GATT_VALUE_PERM_UUID_32 = PERM(UUID_LEN, UUID_32),
    BLE_GATT_VALUE_PERM_UUID_128 = PERM(UUID_LEN, UUID_128),

    // read indication enabled
    BLE_GATT_VALUE_PERM_RI_ENABLE = PERM(RI, ENABLE),

} ble_gatt_value_perm_t;

// GATT service permission
typedef enum
{
    BLE_GATT_SERVICE_PERM_MASK_ALL = 0,

    // Support multi link for one service
    BLE_GATT_SERVICE_PERM_MULTI_LINK = PERM(SVC_MI, ENABLE),

    // Service permission for differernt security level
    BLE_GATT_SERVICE_PERM_NOAUTH = PERM(SVC_AUTH, NO_AUTH),
    BLE_GATT_SERVICE_PERM_UNAUTH = PERM(SVC_AUTH, UNAUTH),
    BLE_GATT_SERVICE_PERM_AUTH = PERM(SVC_AUTH, AUTH),
    BLE_GATT_SERVICE_PERM_SEC_CONN = PERM(SVC_AUTH, SEC_CON),

    // Service uuid length
    BLE_GATT_SERVICE_PERM_UUID_16 = PERM(SVC_UUID_LEN, UUID_16),
    BLE_GATT_SERVICE_PERM_UUID_32 = PERM(SVC_UUID_LEN, UUID_32),
    BLE_GATT_SERVICE_PERM_UUID_128 = PERM(SVC_UUID_LEN, UUID_128),
} ble_gatt_service_perm_t;

// serialize 16 bit primary service uuid
#define SERIAL_UUID_16_PRI_SERVICE SERIAL_UUID_16(ATT_DECL_PRIMARY_SERVICE)

// serialize 16 bit charateristic uuid
#define SERIAL_UUID_16_CHARACTERISTIC SERIAL_UUID_16(ATT_DECL_CHARACTERISTIC)

// serialize 16 bit descriptor of charateristic client config uuid
#define SERIAL_UUID_16_CLIENT_CHAR_CFG SERIAL_UUID_16(ATT_DESC_CLIENT_CHAR_CFG)


#endif // BF0_BLE_GATT_H_


