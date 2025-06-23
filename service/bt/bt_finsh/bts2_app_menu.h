/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_MENU_H_
#define _BTS2_APP_MENU_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    menu_cfg = 0x00,
    menu_start,
    menu_main,

    menu_gen,
    menu_gen_3,
    menu_gen_4,
    menu_gen_4_3,
    menu_gen_5,
    menu_gen_5_1,
    menu_gen_5_4,
    menu_gen_5_7,
    menu_gen_5_8,
    menu_gen_5_a,
    menu_gen_6,
    menu_gen_6_1,
    menu_gen_6_2,
    menu_gen_6_3,
    menu_gen_7,
    menu_gen_8,
    menu_gen_8_1,
    menu_gen_8_2,
    menu_gen_8_4,
    menu_gen_8_5,
    menu_gen_8_7,
    menu_gen_8_8,
    menu_gen_8_9,
    menu_gen_8_b,
    menu_gen_9,
    menu_gen_a,
    menu_gen_b,

    menu_sc_pair,
    menu_sc_passkey_notifi,
    menu_sc_input,
    menu_sc_yesno,
    menu_sc_oobdata,
    menu_hfp_hf,
    menu_hfp_hf_5,

    menu_hfp_hf_a,
    menu_hfp_hf_a_1,
    menu_hfp_hf_a_2,
    menu_hfp_hf_h,
    menu_hfp_hf_l,
    menu_hfp_hf_p,

#ifdef CFG_HFP_AG
    menu_hfp_ag,
    menu_hfp_ag_1,
#endif

#ifdef CFG_AV_SNK
    menu_av_snk,
#endif

#ifdef CFG_AV_SRC
    menu_av_src,
    menu_av_src_bqb,
#endif

#ifdef CFG_VDP_SNK
    menu_vdp_snk,
#endif

#ifdef CFG_VDP_SRC
    menu_vdp_src,
#endif

#ifdef CFG_AVRCP
    menu_avrcp,
    menu_avrcp_a,
#endif

#ifdef CFG_HID
    menu_hid,
#endif

#ifdef CFG_SPP_SRV
    menu_spp_srv,
    menu_spp_srv_0,
    menu_spp_srv_3,
    menu_spp_srv_4,
    menu_spp_srv_5,
    menu_spp_srv_6,
#endif
    menu_l2cap_bqb,
    menu_l2cap_bqb_1,
    menu_l2cap_bqb_2,

    menu_pan_g,

#ifdef CFG_PBAP_CLT
    menu_pbap_c,
    menu_pbap_c_1,
    menu_pbap_c_3,
    menu_pbap_c_4,
    menu_pbap_c_6,
#endif

#ifdef CFG_BR_GATT_SRV
    menu_bt_gatt_srv,
#endif

#ifdef CFG_BT_DIS
    menu_bt_dis,
#endif

#ifdef CFG_BT_L2CAP_PROFILE
    menu_bt_l2cap_profile,
#endif

} menu_st;

/*fn */
void bt_disply_menu(bts2_app_stru *bts2_app_data);
void bt_hdl_menu(bts2_app_stru *bts2_app_data);

#ifdef __cplusplus
}
#endif
#endif

