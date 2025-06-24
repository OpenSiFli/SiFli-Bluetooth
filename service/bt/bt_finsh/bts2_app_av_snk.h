/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_AV_SNK_H_
#define _BTS2_APP_AV_SNK_H_


#include "drivers/bt_device.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef  CFG_AV_SNK

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
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
void bt_avsnk_init(bts2s_av_inst_data *inst, bts2_app_stru *bts2_app_data);
void bt_avsnk_open(bts2s_av_inst_data *inst);
void bt_avsnk_close(bts2s_av_inst_data *inst);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 * INPUT:
 *      void.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bt_avsnk_rel(bts2s_av_inst_data *inst);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
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
bt_err_t bt_a2dp_source_connect_request(BTS2S_BD_ADDR *bd);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
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
void bt_avsnk_abort_stream(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
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
void bt_avsnk_disc_2_src(BOOL is_close);

void bt_avsnk_disc_by_addr(BTS2S_BD_ADDR *bd_addr, BOOL is_close);
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
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
void bt_avsnk_start_stream(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
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
void bt_avsnk_suspend_stream(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
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
void bt_avsnk_release_stream(bts2_app_stru *bts2_app_data);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
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
void bt_avsnk_msg_handler(bts2_app_stru *bts2_app_data);
#else
#define bt_avsnk_init(inst,bts2_app_data)
#define bt_avsnk_open(inst)
#define bt_avsnk_close(inst)
#define bt_avsnk_rel(inst)
#define bt_a2dp_source_connect_request(bts2_app_data) (-BT_ERROR_INPARAM)
#define bt_avsnk_abort_stream(bts2_app_data)
#define bt_avsnk_disc_2_src(is_close)
#define bt_avsnk_start_stream(bts2_app_data)
#define bt_avsnk_suspend_stream(bts2_app_data)
#define bt_avsnk_release_stream(bts2_app_data)
#define bt_avsnk_msg_handler(bts2_app_data)
#endif

#ifdef __cplusplus
}
#endif
#endif

