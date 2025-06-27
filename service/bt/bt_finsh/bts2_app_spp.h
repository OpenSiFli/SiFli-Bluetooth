/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_SPP_B_H_
#define _BTS2_APP_SPP_B_H_
#ifdef __cplusplus
extern "C" {
#endif

#ifdef CFG_SPP_SRV

#define SPP_NOTIFY_RESULT_ACCEPT (0)
#define SPP_NOTIFY_RESULT_REJECT (1)
//!User decides whether to accept the connection or not.
#define SPP_NOTIFY_RESULT_ASYNCHRONOUS_HANDLE (2)

typedef struct
{
    U8 device_id;
    U8 srv_chl;
} bts2_spp_srv_common;



void bt_spp_srv_add_uuid_list(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *      void *msg: message.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_set_instance_index(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *      void *msg: message.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_init(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *      void *msg: message.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_srv_start_enb(bts2_app_stru *bts2_app_data);


void bt_spp_client_start_w4conn(BTS2S_BD_ADDR *bd_addr, U8 *uuid, U8 uuid_len);

void bt_spp_client_start_sdp_search(BTS2S_BD_ADDR *bd_addr, U8 *uuid, U8 uuid_len);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_srv_start_disb(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_mode_change_req(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chnl, U8 mode);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_select_file_to_send(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl, char *file_name);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_open_the_selected_file(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
BOOL bt_spp_check_addr_is_connected(bts2_app_stru *bts2_app_data, BTS2S_BD_ADDR *bd, U8 *idx);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U8 bt_spp_get_available_sub_inst(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bts2_spp_service_list *bt_spp_add_new_service_list(bts2_spp_srv_inst_data *spp_srv_inst_ptr, U8 srv_chl);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_delete_service_list_by_srv_chl(bts2_spp_srv_inst_data *spp_srv_inst_ptr, U8 srv_chl);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
bts2_spp_service_list *bt_spp_get_service_list_by_srv_chl(bts2_spp_srv_inst_data *spp_srv_inst_ptr, U8 srv_chl);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      send data to special channel
 *
 * INPUT:
 *      void *msg: message.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_sending_data_by_device_id_and_srv_chnl(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl, U8 *payload, U16 payload_len);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      get mtu size of special channel
 *
 * INPUT:
 *      void *msg: message.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U16 bt_spp_get_mtu_size(bts2_app_stru *bts2_app_data, U8 device_id);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_sending_the_selected_file(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_sending_next_pieceof_file(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *      U16 U16:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_time_out(U16 spp_id, bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_srv_esc_file_transfer(bts2_app_stru *bts2_app_data, U8 spp_id);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
#ifdef CFG_SPP_LOOPBACK
void bt_spp_set_spp_data_loopback_enable(BOOL enable);
BOOL bt_spp_get_spp_data_loopback_enable(void);
#endif

void bt_spp_data_ind_hdl(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_msg_hdl(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_srv_rfc_conn_accept_hdl(U8 srv_chl, BTS2S_BD_ADDR bd);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_srv_rfc_conn_rej_hdl(U8 srv_chl, BTS2S_BD_ADDR bd);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U8 bt_spp_get_srv_chl_by_uuid(U8 *uuid, U8 uuid_len);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_dump_all_spp_connect_information(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_test_req(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_linest_req(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_switch_role(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_portneg_req(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *      bts2_app_stru *bts2_app_data:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_disc_req(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl);

void bt_spp_disc_req_all(bts2_app_stru *bts2_app_data);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_spp_sending_random_data(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U8 bt_spp_sending_random_data_next(bts2_spp_service_list *spp_service_list, U8 device_id);

void bt_spp_set_write_into_file(bts2_app_stru *bts2_app_data, U8 on_or_off);

void bt_spp_sending_data_to_peer(bts2_app_stru *bts2_app_data, U8 device_id, U8 srv_chl);
#else
#define bt_spp_set_instance_index(bts2_app_data)
#define bt_spp_init(bts2_app_data)
#define bt_spp_srv_start_enb(bts2_app_data)
#define bt_spp_srv_start_disb(bts2_app_data)
#define bt_spp_mode_change_req(bts2_app_data)
#define bt_spp_select_file_to_send(bts2_app_data)
#define bt_spp_open_the_selected_file(bts2_app_data)
#define bt_spp_sending_data_to_peer(bts2_app_data,payload,payload_len)
#define bt_spp_get_mtu_size(bts2_app_data) 0
#define bt_spp_sending_the_selected_file(bts2_app_data,inst_data)
#define bt_spp_sending_next_pieceof_file(bts2_app_data,inst_data)
#define bt_spp_srv_esc_file_transfer(bts2_app_data)
#define bt_spp_data_ind_hdl(bts2_app_data)
#define bt_spp_msg_hdl(bts2_app_data)
#define bt_spp_srv_rfc_conn_accept_hdl()
#define bt_spp_srv_rfc_conn_rej_hdl()
#define bt_spp_test_req(bts2_app_data)
#define bt_spp_linest_req(bts2_app_data)
#define bt_spp_switch_role(bts2_app_data)
#define bt_spp_portneg_req(bts2_app_data)
#define bt_spp_disc_req(bts2_app_data)
#define bt_spp_sending_random_data(bts2_app_data)
#endif
#ifdef __cplusplus
}
#endif
#endif

