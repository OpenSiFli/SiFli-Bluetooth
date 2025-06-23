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

typedef struct
{
    U8 len;
    union
    {
        U16 uuid16;
        U32 uuid32;
        U8 uuid128[LEN_UUID_128];
    } uu;
} GAP_BT_UUID;
#ifdef __cplusplus
}
#endif

#endif
