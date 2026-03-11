/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _GAP_SDC_H_
#define _GAP_SDC_H_

#define LEN_UUID_16 2
#define LEN_UUID_32 4
#define LEN_UUID_128 16

#define LEN_ATTR_ID_8   1
#define LEN_ATTR_ID_16  2
#define LEN_ATTR_ID_32  4

#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{
    GAP_SDPC_PDU_IDLE,
    GAP_SDPC_PDU_SERVICE_SEARCH,
    GAP_SDPC_PDU_SERVICE_ATT_SEARCH,
    GAP_SDPC_PDU_ATT_SEARCH,
} gap_sdc_pdu_t;

typedef struct __attribute__((packed))
{
    U8 len;
    union
    {
        U16 uuid16;
        U32 uuid32;
        U8 uuid128[LEN_UUID_128];
    } uu;
}
GAP_BT_UUID;

typedef struct __attribute__((packed))
{
    U8 len;
    union
    {
        U8 attr_id_8;
        U16 attr_id_16;
        U32 attr_id_32;
    } uu;
}
GAP_BT_ATTR_ID;

#ifdef __cplusplus
}
#endif

#endif
