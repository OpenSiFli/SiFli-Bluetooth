/*
 * SPDX-FileCopyrightText: 2024-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bts2_global.h"
#ifdef CFG_BT_DIS
#include "bts2_app_inc.h"

#define LOG_TAG         "btapp_dis"
#include "log.h"


bt_dis_device_info_t g_dis_info =
{
    .prim_rec = 1,
    .spec_id = 0x0102,
    .prod_id = DIS_PRODUCT_ID,
    .prod_ver = DIS_PRODUCT_VERSION,
    .vend_id = DIS_VENDOR_ID,
    .vend_id_source = DIS_VENDID_SOURCE_BTSIG
};

uint32_t g_sdp_rec_hdl = 0xffff;


void bt_dis_app_set_prim_rec(uint8_t prim_rec)
{
    g_dis_info.prim_rec = prim_rec;
}

void bt_dis_app_set_vendor_id(uint16_t vend_id, uint16_t vend_id_source)
{
    g_dis_info.vend_id = vend_id;
    g_dis_info.vend_id_source = vend_id_source;
}

void bt_dis_app_set_prod_id(uint16_t prod_id)
{
    g_dis_info.prod_id = prod_id;
}

void bt_dis_app_set_prod_ver(uint16_t prod_ver)
{
    g_dis_info.prod_ver = prod_ver;
}

void bt_dis_app_set_spec_id(uint16_t spec_id)
{
    g_dis_info.spec_id = spec_id;
}

void bt_dis_app_set_sdp_rec_hdl(uint32_t sdp_rec_hdl)
{
    g_sdp_rec_hdl = sdp_rec_hdl;
}

bt_dis_device_info_t *bt_dis_app_get_dis_info()
{
    return &g_dis_info;
}

uint8_t bt_dis_app_get_prim_rec()
{
    return g_dis_info.prim_rec;
}

uint16_t bt_dis_app_get_vendor_id()
{
    return g_dis_info.vend_id;
}

uint16_t bt_dis_app_get_vend_id_source()
{
    return g_dis_info.vend_id_source;
}

uint16_t bt_dis_app_get_prod_id()
{
    return g_dis_info.prod_id;
}

uint16_t bt_dis_app_get_prod_ver()
{
    return g_dis_info.prod_ver;
}

uint16_t bt_dis_app_get_spec_id()
{
    return g_dis_info.spec_id;
}

uint32_t bt_dis_app_get_sdp_rec_hdl()
{
    return g_sdp_rec_hdl;
}


bt_err_t bt_dis_app_sdp_reg()
{
    bt_dis_sdp_req_api(bt_dis_app_get_dis_info());
    return BT_EOK;
}

bt_err_t bt_dis_app_sdp_unreg()
{
    bt_dis_sdp_unreq_api(bt_dis_app_get_sdp_rec_hdl());
    return BT_EOK;
}


void bt_dis_app_msg_hdl(bts2_app_stru *bts2_app_data)
{
    bt_dis_sdp_reg_cfm_t *msg;
    msg = (bt_dis_sdp_reg_cfm_t *)bts2_app_data->recv_msg;

    USER_TRACE("bt_dis_app_msg_hdl *msg_type %x\n", msg->type);

    switch (msg->type)
    {
    case BTS2MU_BT_DIS_SDP_RES:
    {
        bt_dis_app_set_sdp_rec_hdl(msg->sdp_sec_hdl);
        break;
    }

    case BTS2MU_BT_DIS_SDP_UNRES:
    {
        bt_dis_app_set_sdp_rec_hdl(msg->sdp_sec_hdl);
        break;
    }

    }
}
#endif

