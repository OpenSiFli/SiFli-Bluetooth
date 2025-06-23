/*
 * SPDX-FileCopyrightText: 2025-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <stdlib.h>

#ifdef A2DP_RELAY_SERVICE

#include "a2dp_relay.h"
#include "bf0_ble_gap.h"
#include "bf0_sibles.h"
#include "att.h"
#include "bf0_sibles_advertising.h"
#include "bf0_sibles_serial_trans_client.h"
#include "av_sbc_api.h"
#include "bt_connection_manager.h"
#include "bts2_app_inc.h"

#define LOG_TAG "a2_rly"

#include "log.h"

typedef enum
{
    A2DP_SRC_STATE_IDLE,
    A2DP_SRC_STATE_CONN,
} A2DP_SRC_STATE_t;



typedef struct
{
    A2DP_RELAY_ROLE_t role;
    uint8_t src_channel;
    A2DP_SRC_STATE_t state;
    uint8_t serial_handle;
    uint8_t conn_idx;
    uint8_t is_scanning;
    uint8_t is_connecting;
    uint8_t is_playing;
    uint8_t codec;
    uint8_t max_playing_cnt;
    a2dp_relay_callback callback;
    a2dp_relay_sbc_para_t sbc_para;
    struct
    {
        bd_addr_t peer_addr;
        uint16_t mtu;
        uint16_t conn_interval;
        uint8_t peer_addr_type;
    } conn_para;
} a2dp_relay_src_env_t;




static a2dp_relay_src_env_t g_a2dp_relay_src_env;



static uint16_t a2dp_command_send(a2dp_relay_src_env_t *env, uint8_t type, uint16_t len, uint8_t *para);
static void a2dp_relay_start_int(a2dp_relay_src_env_t *env);


/********************************************
* Internal APIs
*
*********************************************/

static a2dp_relay_src_env_t *a2dp_relay_src_get_env(void)
{
    return &g_a2dp_relay_src_env;
}

static uint8_t *adv_data_found(uint8_t *p_data, uint8_t type, uint16_t *length)
{
    if (!p_data || !length)
        return NULL;

    // Cursor
    uint8_t *p_cursor = p_data;
    // End of data
    uint8_t *p_end_cursor = p_data + *length;

    while (p_cursor < p_end_cursor)
    {
        // Extract AD type
        uint8_t ad_type = *(p_cursor + 1);

        // Check if it's AD Type which shall be unique
        if (ad_type == type)
        {
            *length = *p_cursor - 1;
            break;
        }

        /* Go to next advertising info */
        p_cursor += (*p_cursor + 1);
    }

    return (p_cursor >= p_end_cursor) ? NULL : (p_cursor + 2);
}

static uint8_t a2dp_relay_src_adv_parser(a2dp_relay_src_env_t *env, ble_gap_ext_adv_report_ind_t *ind)
{
    uint8_t *data = ind->data;
    uint16_t len = ind->length;
    uint8_t svc_uuid[ATT_UUID_128_LEN] = A2DP_RELAY_SVC_UUID;
    uint16_t uuid_len = ATT_UUID_128_LEN;
    uint8_t ret = 0;

    if ((data = adv_data_found(data, BLE_GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID, &uuid_len)) != NULL
            && !env->is_connecting)
    {
        LOG_I("uuid found %x%x%x%x%x%x%x%x%x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
        if (memcmp(data, svc_uuid, ATT_UUID_128_LEN) == 0)
        {
            LOG_I("create connection");
            ble_gap_connection_create_param_t conn_param =
            {
                .own_addr_type = GAPM_STATIC_ADDR,
                .conn_to = 500, // if connection not established in 5s, will cancel this conneciton.
                .type = GAPM_INIT_TYPE_DIRECT_CONN_EST,
                .conn_param_1m.scan_intv = 0x30,
                .conn_param_1m.scan_wd = 0x30,
                .conn_param_1m.conn_intv_max = RELAY_INTERVAL * 4 / 5,
                .conn_param_1m.conn_intv_min = RELAY_INTERVAL * 4 / 5,
                .conn_param_1m.supervision_to = 500,
                .conn_param_1m.ce_len_max = (RELAY_INTERVAL / 2) * 8 / 5,
                .conn_param_1m.ce_len_min = 0,
                .peer_addr = ind->addr,
            };

            if (ble_gap_create_connection(&conn_param) == HL_ERR_NO_ERROR)
            {
                ret = 1;
                env->is_connecting = 1;
            }
            else
                LOG_W("Create con failed");
        }
    }

    return ret;
}

static uint16_t a2dp_data_send(a2dp_relay_src_env_t *env, uint8_t *data, uint16_t len)
{
    return a2dp_command_send(env, A2DP_RELAY_DATA, len, data);
}

static uint16_t a2dp_vol_send(a2dp_relay_src_env_t *env, uint8_t *data, uint16_t len)
{
    return a2dp_command_send(env, A2DP_RELAY_VOL, len, data);
}

static uint16_t a2dp_command_send(a2dp_relay_src_env_t *env, uint8_t type, uint16_t len, uint8_t *para)
{
    uint16_t rlen = sizeof(a2dp_relay_data_t) + len;
    if (env->serial_handle == A2DP_RELAY_INVALID_HDL)
        return 0;

    a2dp_relay_data_t *rdata = malloc(rlen);

    if (rdata == NULL)
        return 0;

    rdata->type = type;
    rdata->len = len;
    memcpy(rdata->data, para, len);


    ble_serial_tran_data_t bdata =
    {
        .handle = env->serial_handle,
        .cate_id = A2DP_RELAY_ID,
        .len = rlen,
        .data = (uint8_t *)rdata,
    };
    ble_serial_tran_error_event_t ret = ble_serial_client_send_data(&bdata, 0);

    free(rdata);

    if (ret == BLE_SERIAL_TRAN_ERROR_NO_ERROR)
        return len;
    else
        return 0;
}

void a2dp_data_src_handler(ble_serial_tran_event_t event, uint8_t *data)
{
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();

    switch (event)
    {
    case BLE_SERIAL_TRAN_OPEN:
    {
        ble_serial_open_t *open = (ble_serial_open_t *)data;

        // only set for 1st device
        if (env->serial_handle == A2DP_RELAY_INVALID_HDL)
        {
            LOG_I("data opened");
            uint8_t role = A2DP_RELAY_SRC;
            env->serial_handle = open->handle;
            env->state = A2DP_SRC_STATE_CONN;
            if (env->is_playing)
            {
                a2dp_relay_start_int(env);
            }

            if (env->callback)
                env->callback(A2DP_RELAY_EVT_PAIRED, &role, 1);

            if (env->is_scanning)
            {
                ble_gap_scan_stop();
            }
        }
        break;
    }
    case BLE_SERIAL_TRAN_OPEN_FAILED:
    {
        // unexpected device, just disconnect
        ble_serial_open_t *open = (ble_serial_open_t *)data;
        ble_gap_disconnect_t dis_conn =
        {
            .conn_idx = open->handle,
            .reason = CO_ERROR_REMOTE_USER_TERM_CON,
        };

        ble_gap_disconnect(&dis_conn);
        break;
    }
    case BLE_SERIAL_TRAN_DATA:
    {
        LOG_W(" BLE SRC received data");
    }
    break;
    case BLE_SERIAL_TRAN_CLOSE:
    {
        ble_serial_close_t *close = (ble_serial_close_t *)data;
        LOG_I("chan closed %d", close->handle);
        if (env->serial_handle == close->handle)
        {
            env->serial_handle = A2DP_RELAY_INVALID_HDL;
            env->state = A2DP_SRC_STATE_IDLE;
        }
        if (env->callback)
            env->callback(A2DP_RELAY_EVT_DISABLED, NULL, 0);
    }
    break;
    case BLE_SERIAL_TRAN_SEND_AVAILABLE:
    {
#if 0
        if (!ctx->buf_cnt && ctx->is_send_playing)
        {
            // release bts thread
            LOG_I("data release");
            rt_mutex_release(g_a2dp_relay_ctx.start_mux);
            ctx->is_send_playing = 0;
        }
#endif
    }
    break;
    case BLE_SERIAL_TRAN_ERROR:
    {
        ble_serial_tran_error_t *err = (ble_serial_tran_error_t *)data;
        LOG_E("src tran err %d", err->error);
    }
    break;
    default:
        break;
    }
}
BLE_SERIAL_CLIENT_EXPORT(A2DP_RELAY_ID, a2dp_data_src_handler);

static int a2dp_relay_ble_event_handler(uint16_t event_id, uint8_t *data, uint16_t len, uint32_t context)
{
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    switch (event_id)
    {
    case BLE_GAP_SCAN_START_CNF:
    {
        ble_gap_start_scan_cnf_t *cnf = (ble_gap_start_scan_cnf_t *)data;
        LOG_I("Scan start status %d", cnf->status);
        if (cnf->status != HL_ERR_NO_ERROR)
        {
            env->is_scanning = 0;
        }
        if (env->callback)
        {
            uint8_t result = env->is_scanning ? A2DP_RELAY_ERR_NO_ERROR : A2DP_RELAY_ERR_GENERAL;
            env->callback(A2DP_RELAY_EVT_ENABLED, &result, 1);
        }
        break;
    }
    case BLE_GAP_SCAN_STOP_CNF:
    {
        ble_gap_stop_scan_cnf_t *cnf = (ble_gap_stop_scan_cnf_t *)data;
        LOG_I("Scan stop");
        break;
    }
    case BLE_GAP_SCAN_STOPPED_IND:
    {
        ble_gap_scan_stopped_ind_t *ind = (ble_gap_scan_stopped_ind_t *)data;
        LOG_I("Scan stopped %d", ind->reason);
        env->is_scanning = 0;
        if (env->state != A2DP_SRC_STATE_CONN)
        {
            if (env->callback)
                env->callback(A2DP_RELAY_EVT_DISABLED, NULL, 0);
        }
        break;
    }
    case BLE_GAP_EXT_ADV_REPORT_IND:
    {
        ble_gap_ext_adv_report_ind_t *ind = (ble_gap_ext_adv_report_ind_t *)data;
        a2dp_relay_src_adv_parser(env, ind);
        break;
    }
    case BLE_GAP_CREATE_CONNECTION_CNF:
    {
        ble_gap_create_connection_cnf_t *cnf = (ble_gap_create_connection_cnf_t *)data;
        if (cnf->status != HL_ERR_NO_ERROR)
        {
            env->is_connecting = 0;
            LOG_E("Create connection failed %d!", cnf->status);
        }
        break;
    }
    case BLE_GAP_CREATE_CONNECTION_STOP_IND:
    {
        ble_gap_create_connection_stop_ind_t *ind = (ble_gap_create_connection_stop_ind_t *)data;
        LOG_I("Create connection stopped: %d", ind->reason);
        if (env->is_connecting)
            env->is_connecting = 0;
        break;
    }
    case BLE_GAP_CONNECTED_IND:
    {
        ble_gap_connect_ind_t *ind = (ble_gap_connect_ind_t *)data;
        env->conn_idx = ind->conn_idx;
        env->conn_para.conn_interval = ind->con_interval;
        env->conn_para.peer_addr_type = ind->peer_addr_type;
        env->conn_para.peer_addr = ind->peer_addr;

        env->is_connecting = 0;

        LOG_I("Peer device(%x-%x-%x-%x-%x-%x) connected", env->conn_para.peer_addr.addr[5],
              env->conn_para.peer_addr.addr[4],
              env->conn_para.peer_addr.addr[3],
              env->conn_para.peer_addr.addr[2],
              env->conn_para.peer_addr.addr[1],
              env->conn_para.peer_addr.addr[0]);

        sibles_exchange_mtu(env->conn_idx);
        break;
    }
    case BLE_GAP_UPDATE_CONN_PARAM_IND:
    {
        ble_gap_update_conn_param_ind_t *ind = (ble_gap_update_conn_param_ind_t *)data;
        env->conn_para.conn_interval = ind->con_interval;
        if (env->role == A2DP_RELAY_SRC
                && env->state == A2DP_SRC_STATE_IDLE)
        {
            LOG_I("open relay src");
            ble_serial_client_open(env->conn_idx);
        }
        LOG_I("Updated connection interval :%d", ind->con_interval);

        break;
    }
    case BLE_GAP_REMOTE_PHY_IND:
    {
        ble_gap_remote_phy_ind_t *phy_ind = (ble_gap_remote_phy_ind_t *)data;
        LOG_I("tx phy %d, rx phy %d", phy_ind->tx_phy, phy_ind->rx_phy);
        if ((env->conn_para.conn_interval != 0 &&
                env->conn_para.conn_interval > RELAY_INTERVAL))
        {
            uint32_t interval = RELAY_INTERVAL * 4 / 5;
            ble_gap_update_conn_param_t conn_para =
            {
                .conn_idx = phy_ind->conn_idx,
                .intv_max = interval,
                .intv_min = interval,
                /* value = argv * 1.25 */
                .ce_len_max = RELAY_INTERVAL / 2,
                .ce_len_min = 0x0,
                .latency = 0,
                .time_out = 500,
            };
            ble_gap_update_conn_param_on_l2cap(&conn_para);
        }
        else
        {
            if (env->role == A2DP_RELAY_SRC
                    && env->state == A2DP_SRC_STATE_IDLE)
            {
                LOG_I("open relay src");
                ble_serial_client_open(env->conn_idx);
            }
        }
        break;
    }
    case BLE_GAP_UPDATE_DATA_LENGTH_IND:
    {
        ble_gap_update_data_length_ind_t *ind = (ble_gap_update_data_length_ind_t *)data;
        ble_gap_update_phy_t phy =
        {
            .conn_idx = ind->conn_idx,
            .tx_phy = GAP_PHY_LE_2MBPS,
            .rx_phy = GAP_PHY_LE_2MBPS,
            .phy_opt = 0,
        };
        ble_gap_update_phy(&phy);
        break;
    }
    case SIBLES_MTU_EXCHANGE_IND:
    {
        /* Negotiated MTU. */
        sibles_mtu_exchange_ind_t *ind = (sibles_mtu_exchange_ind_t *)data;
        env->conn_para.mtu = ind->mtu;
        LOG_I("Exchanged MTU size: %d", ind->mtu);
        break;
    }
    case BLE_GAP_DISCONNECTED_IND:
    {
        ble_gap_disconnected_ind_t *ind = (ble_gap_disconnected_ind_t *)data;
        env->conn_idx = 0xFF;
        LOG_I("BLE_GAP_DISCONNECTED_IND(%d)", ind->reason);
        break;
    }
    default:
        break;
    }
    return 0;
}
BLE_EVENT_REGISTER(a2dp_relay_ble_event_handler, NULL);


/******************************************
 * Implement weak func for bts2_av_snk.c
 *
******************************************/

uint16_t a2dp_relay_transfer(uint8_t *data, uint16_t len, uint8_t left_cnt, uint32_t left_byte)
{
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    uint32_t *left_ms_ptr = (uint32_t *)data;
    *left_ms_ptr = left_byte;
    data[4] = left_cnt;


    return a2dp_data_send(env, data, len);
}

void a2dp_relay_trigger_audio_server(void)
{
    //rt_mutex_take(g_a2dp_relay_ctx.start_mux, RT_WAITING_FOREVER);
}


static void a2dp_relay_start_int(a2dp_relay_src_env_t *env)
{
    uint8_t init_para[11];
    init_para[0] = env->codec;
    init_para[1] = env->sbc_para.chnl_mode;
    init_para[2] = env->sbc_para.alloc_method;
    init_para[3] = env->sbc_para.sample_freq & 0xFF;
    init_para[4] = (env->sbc_para.sample_freq >> 8) & 0xFF;
    init_para[5] = env->sbc_para.blocks;
    init_para[6] = env->sbc_para.subbands;
    init_para[7] = env->sbc_para.bitpool;
    init_para[8] = 0;
    init_para[9] = env->max_playing_cnt;
    init_para[10] = env->src_channel < 2 ? (env->src_channel == 0 ? 1 : 0) : 2;

    a2dp_command_send(env, A2DP_RELAY_START, 11, init_para);
}

void a2dp_relay_start(U8 codec, BTS2E_SBC_CHNL_MODE chnl_mode, BTS2E_SBC_ALLOC_METHOD alloc_method, U16 sample_freq,
                      U8 nrof_blocks, U8 nrof_subbands, U8 bitpool,  U8 start_playing_cnt, U8 max_playing_cnt, U8 target_channel)
{
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    env->sbc_para.blocks = nrof_blocks;
    env->sbc_para.subbands = nrof_subbands;
    env->sbc_para.chnl_mode = chnl_mode;
    env->sbc_para.alloc_method = alloc_method;
    env->sbc_para.bitpool = bitpool;
    env->sbc_para.sample_freq = sample_freq;
    env->codec = codec;
    env->max_playing_cnt = max_playing_cnt;
    env->sbc_para.blocks = nrof_blocks;

    LOG_I("start state %d", env->state);
    env->is_playing = 1;
    if (env->state == A2DP_SRC_STATE_CONN)
        a2dp_relay_start_int(env);

}

void a2dp_relay_stop(void)
{
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    LOG_I("stop state %d", env->state);
    if (env->is_playing)
        env->is_playing = 0;

    if (env->state == A2DP_SRC_STATE_CONN)
        a2dp_command_send(env, A2DP_RELAY_STOP, 0, NULL);
}

uint8_t a2dp_relay_stereo_enable(void)
{
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    return env->src_channel < 2 ? 1 : 0;
}

uint8_t a2dp_relay_get_channel(void)
{
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    return env->src_channel;
}


/* ************************************
 *  Public APIs
 *
 **************************************/

void a2dp_relay_src_init(a2dp_relay_callback callback)
{
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    env->role = A2DP_RELAY_NO_ROLE;
    env->serial_handle = A2DP_RELAY_INVALID_HDL;
    env->src_channel = 2;
    env->callback = callback;

    ble_serial_client_init();


}

void a2dp_relay_src_role_set(uint8_t role)
{
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    env->role = role;
}

// 0 is left chanel, 1 is right channel, 2 is stereo
uint8_t a2dp_relay_src_channel_set(uint8_t channel)
{
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    uint8_t ret = A2DP_RELAY_ERR_INVALID;
    if (env->state == A2DP_SRC_STATE_IDLE
            && channel <= 2)
    {
        env->src_channel = channel;
        ret = A2DP_RELAY_ERR_NO_ERROR;
    }
    return ret;
}

uint8_t a2dp_relay_vol_set(uint8_t vol)
{
    uint8_t ret = A2DP_RELAY_ERR_INVALID;
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    if (env->state == A2DP_SRC_STATE_CONN)
    {
        if (a2dp_vol_send(env, &vol, 1) >= 0)
            ret = A2DP_RELAY_ERR_NO_ERROR;
    }
    return ret;
}

uint8_t a2dp_relay_src_enable(void)
{
    uint8_t ret = A2DP_RELAY_ERR_GENERAL;
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    do
    {
        if (env->state == A2DP_SRC_STATE_CONN)
        {
            ret = A2DP_RELAY_ERR_ALREADY_CONN;
            break;
        }

        if (env->is_connecting || env->is_scanning)
        {
            ret = A2DP_RELAY_ERR_UNDER_RUN;
            break;
        }

        // Try find relay sink device
        ble_gap_scan_start_t scan_param =
        {
            .own_addr_type = GAPM_STATIC_ADDR,
            .type = GAPM_SCAN_TYPE_OBSERVER,
            .dup_filt_pol = 1,
            .scan_param_1m.scan_intv = A2DP_RELAY_SRC_SEARCH_INTERVAL * 8 / 5,
            .scan_param_1m.scan_wd = A2DP_RELAY_SRC_SEARCH_WINDOW * 8 / 5,
            .duration = A2DP_RELAY_SRC_SEARCH_DURATION * 100,
        };

        if (ble_gap_scan_start(&scan_param) != HL_ERR_NO_ERROR)
            break;

        env->role = A2DP_RELAY_SRC;
        env->is_scanning = 1;
        ret = A2DP_RELAY_ERR_NO_ERROR;

    }
    while (0);

    return ret;
}

uint8_t a2dp_relay_src_disable(void)
{
    uint8_t ret = A2DP_RELAY_ERR_NO_ERROR;
    a2dp_relay_src_env_t *env = a2dp_relay_src_get_env();
    do
    {
        if (env->state ==  A2DP_SRC_STATE_CONN)
        {
            ble_gap_disconnect_t dis =
            {
                .conn_idx = env->conn_idx,
                .reason = CO_ERROR_REMOTE_USER_TERM_CON,
            };
            ble_gap_disconnect(&dis);
            break;
        }

        if (env->is_connecting || env->is_scanning)
        {
            if (env->is_connecting)
                ble_gap_cancel_create_connection();

            if (env->is_scanning)
                ble_gap_scan_stop();

            break;
        }

        ret = A2DP_RELAY_ERR_INVALID;

    }
    while (0);

    return ret;
}


#endif // A2DP_RELAY_SERVICE

