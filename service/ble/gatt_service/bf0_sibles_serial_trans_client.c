/*
 * SPDX-FileCopyrightText: 2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdlib.h>
#include "os_adaptor.h"
#include "bf0_ble_gap.h"
#include "bf0_sibles.h"
#include "bf0_sibles_internal.h"
#include "bf0_sibles_serial_trans_service.h"
#include "bf0_sibles_serial_trans_client.h"
#include "att.h"
#include "section.h"
#include "ble_connection_manager.h"

#ifdef BSP_BLE_SERIAL_TRANSMISSION

#define LOG_TAG "sibles"
#include "log.h"

typedef enum
{
    BLE_SERIAL_CLIENT_STATE_IDLE,
    BLE_SERIAL_CLIENT_STATE_SEARCHING,
    BLE_SERIAL_CLIENT_STATE_SEARCH_COMPLETED,
    BLE_SERIAL_CLIENT_STATE_CONFIGURAING,
    BLE_SERIAL_CLIENT_STATE_READY
} ble_serial_client_state_t;

typedef struct
{
    uint8_t *ptr;
    uint16_t len;
    uint16_t offset;
    uint8_t id;
} ble_serial_tran_assemable_t;

typedef struct
{
    uint8_t hdl_start;
    uint8_t hdl_end;
} ble_serial_client_svc_t;

typedef struct
{
    uint16_t attr_hdl;
    uint16_t value_hdl;
    uint8_t prop;
    uint16_t cccd_hdl;
} ble_serial_client_char_t;

typedef struct
{
    ble_serial_client_svc_t svc;
    ble_serial_client_char_t config_char;
    ble_serial_client_char_t data_char;
    uint8_t cb_count;
    uint8_t is_assemable;
    uint16_t mtu;
    ble_serial_tran_export_t *cb_table;
    ble_serial_tran_assemable_t assemable;
    uint8_t conn_idx;
    uint8_t is_cccd_on;
    uint8_t is_init;
    uint16_t remote_handle;
    ble_serial_client_state_t state;
} ble_serial_client_env_t;

static const uint8_t g_serial_tran_svc_uuid[ATT_UUID_128_LEN] = serial_tran_svc_uuid;
static const uint8_t g_serial_tran_configure_uuid[ATT_UUID_128_LEN] = serial_tran_configure_uuid;
static const uint8_t g_serial_tran_data_uuid[ATT_UUID_128_LEN] = serial_tran_data_uuid;

static ble_serial_client_env_t g_ble_serial_client_env;

static ble_serial_client_env_t *ble_serial_client_get_env(void)
{
    return &g_ble_serial_client_env;
}

static uint32_t ble_serial_wait_time_get(uint8_t conn_idx)
{
    cm_conneciont_parameter_value_t con_para;
    uint32_t wait_time;
    uint8_t para_ret = connection_manager_get_connetion_parameter(conn_idx, (uint8_t *)&con_para);
    if (para_ret == CM_STATUS_OK && con_para.interval != 0)
    {
        wait_time = con_para.interval * 1.25f;
    }
    else
    {
        wait_time = 50;
    }
    return wait_time;
}




static void ble_serial_client_callback_error_notify(uint8_t conn_idx, uint8_t cate_id, uint8_t error)
{
    ble_serial_client_env_t *env = ble_serial_client_get_env();
    ble_serial_tran_export_t *tb = env->cb_table;

    uint32_t i;
    for (i = 0; i < env->cb_count; i++)
    {
        if (cate_id == tb->cate_id)
        {
            LOG_W("send error notify error:%x,id:%d", error, tb->cate_id);
            ble_serial_tran_error_t data;
            data.handle = conn_idx;
            data.cate_id = cate_id;
            data.error = error;
            tb->callback(BLE_SERIAL_TRAN_ERROR, (uint8_t *)&data);
            break;
        }
        tb++;
    }
}

static void ble_serial_callback_data_notify(uint8_t conn_idx, uint8_t cate_id, uint16_t len, uint8_t *value)
{
    ble_serial_client_env_t *env = ble_serial_client_get_env();

    if (!value)
        return;
    uint32_t i;
    ble_serial_tran_export_t *tb = env->cb_table;
    LOG_D("client cb_id %d", cate_id);
    // value + 1 is fragment flag
    for (i = 0; i < env->cb_count; i++)
    {
        LOG_D("client tb cb:%x,id:%d", tb->callback, tb->cate_id);
        if (cate_id == tb->cate_id)
        {
            ble_serial_tran_data_t data;
            data.handle = conn_idx;
            data.cate_id = cate_id;
            data.len = len;
            data.data = value;
            tb->callback(BLE_SERIAL_TRAN_DATA, (uint8_t *)&data);
            break;
        }
        tb++;
    }
}

static void ble_serial_client_callback_data_notify(uint8_t conn_idx, uint8_t cate_id, uint16_t len, uint8_t *value)
{
    ble_serial_client_env_t *env = ble_serial_client_get_env();

    if (!value)
        return;
    uint32_t i;
    ble_serial_tran_export_t *tb = env->cb_table;
    LOG_D("client cb_id %d", cate_id);
    // value + 1 is fragment flag
    for (i = 0; i < env->cb_count; i++)
    {
        LOG_D("client tb cb:%x,id:%d", tb->callback, tb->cate_id);
        if (cate_id == tb->cate_id)
        {
            ble_serial_tran_data_t data;
            data.handle = conn_idx;
            data.cate_id = cate_id;
            data.len = len;
            data.data = value;
            tb->callback(BLE_SERIAL_TRAN_DATA, (uint8_t *)&data);
            break;
        }
        tb++;
    }
}

static void ble_serial_client_callback_event_notify(ble_serial_tran_event_t event, uint8_t *data)
{
    ble_serial_client_env_t *env = ble_serial_client_get_env();

    if (!data)
        return;
    uint32_t i;
    ble_serial_tran_export_t *tb = env->cb_table;
    for (i = 0; i < env->cb_count; i++)
    {
        LOG_D("client tb cb:%x,id:%d", tb->callback, tb->cate_id);
        if (tb->callback)
            tb->callback(event, data);
        tb++;
    }
}

static void ble_serial_clean_assemble(void)
{
    LOG_I("ble_serial_clean_assemble");
    ble_serial_client_env_t *env = ble_serial_client_get_env();
    env->is_assemable = 0;
    if (env->assemable.ptr)
    {
        bt_mem_free(env->assemable.ptr);
    }
    memset(&env->assemable, 0, sizeof(ble_serial_tran_assemable_t));
}

static int ble_serial_client_send_data_int(uint8_t conn_idx, uint16_t hdl, uint16_t len, uint8_t *data, uint8_t is_retry)
{
    int ret;
    ble_serial_client_env_t *env = ble_serial_client_get_env();
    uint32_t wait_time;
    sibles_write_remote_value_t value;
    value.write_type = SIBLES_WRITE_WITHOUT_RSP;
    value.handle = hdl;
    value.len = len;
    value.value = data;

    do
    {
        ret = sibles_write_remote_value(env->remote_handle, conn_idx, &value);
        if (ret == SIBLES_WRITE_NO_ERR)
        {
            ret = 0;
            break;
        }
        else if (ret == SIBLES_WIRTE_TX_FLOWCTRL_ERR)
        {
            // tx queue is full, wait and retry
            ret = -1;
            if (is_retry)
            {
                int retry = 20;
                while (retry > 0)
                {
                    retry--;
                    wait_time = ble_serial_wait_time_get(conn_idx);
                    rt_thread_mdelay(wait_time);
                    ret = sibles_write_remote_value(env->remote_handle, conn_idx, &value);
                    if (ret == SIBLES_WRITE_NO_ERR)
                    {
                        LOG_I("send retry success");
                        break;
                    }
                }

                if (ret != SIBLES_WRITE_NO_ERR)
                {
                    LOG_E("send fail");
                    ret = -2;
                }
            }

        }
    }
    while (0);

    return ret;
}


static void ble_serail_client_data_handler(ble_serial_client_env_t *env, sibles_remote_event_ind_t *para)
{
    // should get the conn_idx from parameter
    uint8_t frag_flag = *(para->value + 1);
    uint8_t cate_id = *(para->value);
    do
    {
        if (frag_flag != 0 && !env->is_assemable)
        {
            if (frag_flag != 1)
            {
                // only assemable when flag is 1
                break;
            }
            // start assemable
            env->is_assemable = 1;
            memcpy(&env->assemable.len, para->value + 2, sizeof(uint16_t));
            env->assemable.ptr = bt_mem_alloc(env->assemable.len);
            env->assemable.id = cate_id;
            // OS_ASSERT(env->assemable.ptr);
            if (!env->assemable.ptr)
            {
                ble_serial_client_callback_error_notify(para->conn_idx, env->assemable.id, BLE_SERIAL_TRAN_ERROR_LACK_OF_MEMORY);
                ble_serial_clean_assemble();
                break;
            }

            memcpy(env->assemable.ptr, para->value + 4, para->length - 4);
            env->assemable.offset = para->length - 4;
            break;
        }
        else if (frag_flag != 0 && env->is_assemable)
        {
            if (cate_id != env->assemable.id || !env->assemable.ptr)
            {
                //Just handle one uuid asseamblly.
                // OS_ASSERT(0);
                ble_serial_client_callback_error_notify(para->conn_idx, env->assemable.id, BLE_SERIAL_TRAN_ERROR_CATE_ID);
                ble_serial_clean_assemble();
                break;
            }

            if ((env->assemable.offset + (para->length - 2)) > env->assemable.len)
            {
                LOG_E("assemable over len %d, %d", env->assemable.offset + (para->length - 2), env->assemable.len);
                ble_serial_client_callback_error_notify(para->conn_idx, env->assemable.id, BLE_SERIAL_TRAN_ERROR_ASSEMBLE_OVER_LEN);
                ble_serial_clean_assemble();
                break;
            }
            // assemabling
            memcpy(env->assemable.ptr + env->assemable.offset, para->value + 2, para->length - 2);
            env->assemable.offset += para->length - 2;
            if (frag_flag == 3)
            {

                if (env->assemable.offset != env->assemable.len)
                {
                    LOG_E("assemable error len %d, %d", env->assemable.offset, env->assemable.len);
                    ble_serial_client_callback_error_notify(para->conn_idx, env->assemable.id, BLE_SERIAL_TRAN_ERROR_ASSEMBLE_ERROR);
                    ble_serial_clean_assemble();
                    break;
                }

                ble_serial_client_callback_data_notify(para->conn_idx, env->assemable.id, env->assemable.len, env->assemable.ptr);
                bt_mem_free(env->assemable.ptr);
                memset(&env->assemable, 0, sizeof(ble_serial_tran_assemable_t));
                env->is_assemable = 0;
            }
        }
        else if (frag_flag == 0)
        {
            if (env->is_assemable)
            {
                if (env->assemable.id != cate_id)
                {
                    //just notify user
                }
                else
                {
                    // discard last packet
                    env->is_assemable = 0;
                    if (env->assemable.ptr)
                        bt_mem_free(env->assemable.ptr);
                    memset(&env->assemable, 0, sizeof(ble_serial_tran_assemable_t));
                }
            }
        }
        ble_serial_client_callback_data_notify(para->conn_idx, cate_id, para->length - 4, para->value + 4);

    }
    while (0);
}


static int ble_serial_client_gattc_event_handler(uint16_t event_id, uint8_t *data, uint16_t len)
{
    ble_serial_client_env_t *env = ble_serial_client_get_env();
    int8_t  res;

    //LOG_I("ancs gattc event handler %d\r\n", event_id);
    switch (event_id)
    {
    case SIBLES_REGISTER_REMOTE_SVC_RSP:
    {
        sibles_register_remote_svc_rsp_t *rsp = (sibles_register_remote_svc_rsp_t *)data;
        LOG_I("register ret %d\r\n", rsp->status);
        ble_serial_open_t chan;
        chan.handle = rsp->conn_idx;

        if (rsp->status == HL_ERR_NO_ERROR)
        {
            ble_serial_client_callback_event_notify(BLE_SERIAL_TRAN_OPEN, (uint8_t *)&chan);
            //TODO: enable CCCD
            env->state = BLE_SERIAL_CLIENT_STATE_READY;
        }
        else
        {
            ble_serial_client_callback_event_notify(BLE_SERIAL_TRAN_OPEN_FAILED, (uint8_t *)&chan);
            env->state = BLE_SERIAL_CLIENT_STATE_IDLE;
        }

        break;
    }
    case SIBLES_REMOTE_EVENT_IND:
    {
        if (env->state != BLE_SERIAL_CLIENT_STATE_READY)
        {
            LOG_I("ble serial client state error %d", env->state);
            return 0;
        }

        sibles_remote_event_ind_t *ind = (sibles_remote_event_ind_t *)data;
        //LOG_I("ancs handle:%d", ind->handle);
        if (ind->handle == env->data_char.value_hdl)
        {
            ble_serail_client_data_handler(env, ind);
        }
        // Notify upper layer
        break;
    }
    case SIBLES_WRITE_REMOTE_VALUE_RSP:
    {
        // notify upper
        break;
    }
    default:
        break;
    }
    return 0;
}

uint8_t ble_serial_client_open(uint8_t conn_idx)
{
    ble_serial_client_env_t *env = ble_serial_client_get_env();
    if (env->state >= BLE_SERIAL_CLIENT_STATE_SEARCHING)
        return BLE_SERIAL_TRAN_ERROR_INVALID_PARA;
    sibles_search_service(conn_idx, ATT_UUID_128_LEN, (uint8_t *)g_serial_tran_svc_uuid);
    // Only treat enable conn as search target
    env->conn_idx = conn_idx;
    env->state = BLE_SERIAL_CLIENT_STATE_SEARCHING;
    return BLE_SERIAL_TRAN_ERROR_NO_ERROR;
}

ble_serial_tran_error_event_t ble_serial_client_send_data(ble_serial_tran_data_t *data, uint8_t is_retry)
{
    int ret = 0;
    ble_serial_tran_error_event_t res = BLE_SERIAL_TRAN_ERROR_NO_ERROR;
    ble_serial_client_env_t *env = ble_serial_client_get_env();
    uint8_t *packet ;

    if (data == NULL || data->data == NULL)
        return BLE_SERIAL_TRAN_ERROR_INVALID_PARA;                                  // Parameter error;

    if (data->len <= env->mtu - 3 - 4)
    {
        if ((packet = bt_mem_alloc(data->len + 4)) == NULL)
            return BLE_SERIAL_TRAN_ERROR_LACK_OF_MEMORY;                                  // No enough memory
        *packet = data->cate_id;                        // Add cateID to packet
        *(packet + 1) = 0;                              // first not consider fragment
        memcpy(packet + 2, &data->len, 2);
        memcpy(packet + 4, data->data, data->len);

        ret = ble_serial_client_send_data_int(data->handle, env->data_char.value_hdl, data->len + 4, packet, is_retry);
        if (ret < 0)
        {
            LOG_E("unexpect fail %d", ret);
            res = BLE_SERIAL_TRAN_ERROR_COMMON_ERROR;
        }
        bt_mem_free(packet);
        return res;
    }
    else
    {
        // use fragment packet
        uint16_t max_packet_len = env->mtu - 3;
        uint16_t single_packet_len = max_packet_len - 4;

        packet = bt_mem_alloc(max_packet_len);
        if (!packet)
            return BLE_SERIAL_TRAN_ERROR_LACK_OF_MEMORY;

        *packet = data->cate_id;                        // Add cateID to packet
        uint16_t offset = 0;
        uint16_t tran_len;

        while (offset < data->len)
        {
            do
            {
                // first packet
                if (offset == 0)
                {
                    *(packet + 1) = 1;                              // first not consider fragment
                    memcpy(packet + 2, &data->len, 2);
                    memcpy(packet + 4, data->data, single_packet_len);
                    tran_len = single_packet_len + 4;
                    break;
                }
                else if (data->len - offset <= single_packet_len)
                {
                    // last packet
                    single_packet_len = data->len - offset;
                    *(packet + 1) = 3;
                    memcpy(packet + 2, data->data + offset, single_packet_len);
                    tran_len = single_packet_len + 2;
                    break;
                }
                else
                {
                    // continue packet
                    *(packet + 1) = 2;
                    memcpy(packet + 2, data->data + offset, single_packet_len);
                    tran_len = single_packet_len + 2;
                    break;
                }
            }
            while (0);

            ret = ble_serial_client_send_data_int(data->handle, env->data_char.value_hdl, tran_len, packet, is_retry);
            if (ret < 0)
            {
                LOG_E("send frag fail %d", ret);
                res = BLE_SERIAL_TRAN_ERROR_COMMON_ERROR;
            }

            if (res != BLE_SERIAL_TRAN_ERROR_NO_ERROR)
            {
                if (packet)
                    bt_mem_free(packet);

                return res;
            }

            offset += single_packet_len;
        }

        if (packet)
            bt_mem_free(packet);
        return res;
    }
}

int ble_serial_client_event_handler(uint16_t event_id, uint8_t *data, uint16_t len, uint32_t context)
{
    ble_serial_client_env_t *env = ble_serial_client_get_env();
    switch (event_id)
    {
    case BLE_POWER_ON_IND:
    {
        env->conn_idx = INVALID_CONN_IDX;
        break;
    }
    case BLE_GAP_CONNECTED_IND:
    {
        ble_gap_connect_ind_t *ind = (ble_gap_connect_ind_t *)data;
        env->mtu = 23;
        break;
    }
    case SIBLES_MTU_EXCHANGE_IND:
    {
        sibles_mtu_exchange_ind_t *ind = (sibles_mtu_exchange_ind_t *)data;
        env->mtu = ind->mtu;
        break;
    }
    case BLE_GAP_DISCONNECTED_IND:
    {
        ble_gap_disconnected_ind_t *ind = (ble_gap_disconnected_ind_t *)data;
        // clear assemable env if disconnect
        env->is_assemable = 0;
        if (env->assemable.ptr)
        {
            bt_mem_free(env->assemable.ptr);
        }
        memset(&env->assemable, 0, sizeof(ble_serial_tran_assemable_t));

        sibles_unregister_remote_svc(env->conn_idx, env->svc.hdl_start, env->svc.hdl_end, ble_serial_client_gattc_event_handler);
        if (env->state == BLE_SERIAL_CLIENT_STATE_READY)
        {
            ble_serial_close_t chan;
            chan.handle = ind->conn_idx;
            ble_serial_client_callback_event_notify(BLE_SERIAL_TRAN_CLOSE, (uint8_t *)&chan);
        }

        env->state = BLE_SERIAL_CLIENT_STATE_IDLE;
        break;
    }
    case SIBLES_SEARCH_SVC_RSP:
    {
        sibles_svc_search_rsp_t *rsp = (sibles_svc_search_rsp_t *)data;

        // rsp->svc may null
        if (memcmp(rsp->search_uuid, g_serial_tran_svc_uuid, rsp->search_svc_len) != 0)
            break;

        if (rsp->result != HL_ERR_NO_ERROR)
        {
            LOG_E("Serial client search failed");
            ble_serial_open_t chan;
            chan.handle = rsp->conn_idx;
            ble_serial_client_callback_event_notify(BLE_SERIAL_TRAN_OPEN_FAILED, (uint8_t *)&chan);
            env->state = BLE_SERIAL_CLIENT_STATE_IDLE;
            break; // Do nothing
        }

        env->svc.hdl_start = rsp->svc->hdl_start;
        env->svc.hdl_end = rsp->svc->hdl_end;
        uint32_t i;
        uint16_t offset = 0;
        sibles_svc_search_char_t *chara = (sibles_svc_search_char_t *)rsp->svc->att_db;
        for (i = 0; i < rsp->svc->char_count; i++)
        {
            if (!memcmp(chara->uuid, g_serial_tran_configure_uuid, chara->uuid_len))
            {
                LOG_I("noti_uuid received, att handle(%x), des handle(%x)", chara->attr_hdl, chara->desc[0].attr_hdl);
                env->config_char.attr_hdl = chara->attr_hdl;
                env->config_char.value_hdl = chara->pointer_hdl;
                env->config_char.prop = chara->prop;

            }
            else if (!memcmp(chara->uuid, g_serial_tran_data_uuid, chara->uuid_len))
            {
                LOG_I("control received, att handle(%x)", chara->attr_hdl);
                env->data_char.attr_hdl = chara->attr_hdl;
                env->data_char.value_hdl = chara->pointer_hdl;
                env->data_char.prop = chara->prop;
                env->data_char.cccd_hdl = sibles_descriptor_handle_find(chara, ATT_DESC_CLIENT_CHAR_CFG);
            }

            offset = sizeof(sibles_svc_search_char_t) + chara->desc_count * sizeof(struct sibles_disc_char_desc_ind);
            chara = (sibles_svc_search_char_t *)((uint8_t *)chara + offset);
        }

        env->state = BLE_SERIAL_CLIENT_STATE_SEARCH_COMPLETED;
        //register first
        env->remote_handle = sibles_register_remote_svc(rsp->conn_idx, env->svc.hdl_start, env->svc.hdl_end, ble_serial_client_gattc_event_handler);
        // subscribe data src. then subscribe notfi src.
        break;
    }
    case SIBLES_WRITE_REMOTE_VALUE_RSP:
    {
        ble_serial_client_callback_event_notify(BLE_SERIAL_TRAN_SEND_AVAILABLE, NULL);
        break;
    }
    default:
        break;
    }
    return 0;
}
BLE_EVENT_REGISTER(ble_serial_client_event_handler, NULL);

SECTION_DEF(SerialTranClientExport, ble_serial_tran_export_t);

void ble_serial_client_init(void)
{

    ble_serial_client_env_t *env = ble_serial_client_get_env();
    if (env->is_init)
        return;

    env->is_init = 1;
    // Init callback table
    env->cb_table = (ble_serial_tran_export_t *)SECTION_START_ADDR(SerialTranClientExport);
    env->cb_count = (ble_serial_tran_export_t *)SECTION_END_ADDR(SerialTranClientExport) - env->cb_table;
}


#endif // BSP_BLE_SERIAL_TRANSMISSION

