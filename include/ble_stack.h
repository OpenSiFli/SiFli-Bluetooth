/*
 * SPDX-FileCopyrightText: 2020-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BLE_STACK_H
#define __BLE_STACK_H

#if defined(BSP_BLE_SIBLES) && !defined(BSP_USING_PC_SIMULATOR)
    #include "bf0_sibles_util.h"
#endif

#include <rtconfig.h>
#include <stdint.h>

#ifdef SOC_SF32LB55X
typedef struct
{
    uint8_t *env_buf;
    uint8_t *msg_buf;
    uint8_t *att_buf;
    uint8_t *nt_buf;
    uint8_t *log_buf;
    uint16_t env_buf_size;
    uint16_t msg_buf_size;
    uint16_t att_buf_size;
    uint16_t nt_buf_size;
    uint16_t log_buf_size;
    int8_t max_nb_of_hci_completed;
} ble_mem_config_t;
#else
typedef struct
{
    uint8_t *env_buf;
    uint8_t *msg_buf;
    uint8_t *att_buf;
    uint8_t *nt_buf;
    uint8_t *log_buf;
    uint8_t *nvds_buf;
// only defined in rom,and ROM_CONFIG_V2 is not config by mem_config API. Avoid influence on previous chips
#ifdef ROM_CONFIG_V2
    uint8_t *aud_buf;
#endif
    uint16_t env_buf_size;
    uint16_t msg_buf_size;
    uint16_t att_buf_size;
    uint16_t nt_buf_size;
    uint16_t log_buf_size;
    uint16_t nvds_buf_size;
#ifdef ROM_CONFIG_V2
    uint16_t aud_buf_size;
#endif
    int8_t max_nb_of_hci_completed;
} ble_mem_config_t;
#endif

// Order should not be changed
typedef enum
{
    ROM_EM_BLE_START_OFFSET = 0x0,
    ROM_EM_BLE_CS_OFFSET,
    ROM_EM_BLE_WPAL_OFFSET,
    ROM_EM_BLE_RAL_OFFSET,
    ROM_EM_BLE_RX_DESC_OFFSET,
    ROM_EM_BLE_TX_DESC_OFFSET,
    ROM_EM_BLE_LLCPTXBUF_OFFSET,
    ROM_EM_BLE_ADVEXTHDRTXBUF_OFFSET,
    ROM_EM_BLE_ADVDATATXBUF_OFFSET,
    ROM_EM_BLE_AUXCONNECTREQTXBUF_OFFSET,
    ROM_EM_BLE_DATARXBUF_OFFSET,
    ROM_EM_BLE_ACLTXBUF_OFFSET,
    ROM_EM_BLE_ISO_HOP_SEQ_OFFSET,
    ROM_EM_BLE_ISO_DESC_OFFSET,
    ROM_EM_BLE_ISO_BUF_OFFSET,
    ROM_EM_BLE_RX_CTE_DESC_OFFSET,
    ROM_EM_BLE_TX_ANTENNA_ID_OFFSET,
    ROM_EM_BLE_RX_ANTENNA_ID_OFFSET,
    ROM_EM_BLE_END,
    ROM_EM_BT_CS_OFFSET,
    ROM_EM_BT_RXDESC_OFFSET,
    ROM_EM_BT_TXDESC_OFFSET,
    ROM_EM_BT_LMPRXBUF_OFFSET,
    ROM_EM_BT_LMPTXBUF_OFFSET,
    ROM_EM_BT_ISCANFHSTXBUF_OFFSET,
    ROM_EM_BT_PAGEFHSTXBUF_OFFSET,
    ROM_EM_BT_EIRTXBUF_OFFSET,
    ROM_EM_BT_LOCAL_SAM_SUBMAP_OFFSET,
    ROM_EM_BT_PEER_SAM_MAP_OFFSET,
    ROM_EM_BT_STPTXBUF_OFFSET,
    ROM_EM_BT_ACLRXBUF_OFFSET,
    ROM_EM_BT_ACLTXBUF_OFFSET,
    ROM_EM_BT_AUDIOBUF_OFFSET,
    ROM_EM_BT_END,
    ROM_EM_END
} rom_em_offset_t;

typedef struct
{
    uint16_t size;
    uint16_t cnt;
} rom_em_default_attr_t;


typedef int32_t (*sifli_msg_func_t)(uint16_t const msgid, void const *param,
                                    uint16_t const dest_id, uint16_t const src_id);

typedef int (*tl_data_callback_t)(uint8_t *bufptr, uint32_t size);


int bluetooth_init(void);

void ble_boot(sifli_msg_func_t callback);

#if defined(BSP_BLE_SIBLES) && !defined(BSP_USING_PC_SIMULATOR)
void *ble_stack_msg_alloc(sifli_msg_id_t const id, sifli_task_id_t const dest_id,
                          sifli_task_id_t const src_id, uint16_t const param_len);

void ble_stack_msg_send(void const *param_ptr);
#endif

void bluetooth_stack_ipc_register_tx_cb(tl_data_callback_t callback);

int bluetooth_stack_ipc_write_data(uint8_t *bufptr, uint32_t size);

void bluetooth_config(void);


#endif // __BLE_STACK_H




