/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SDAP_API_H_
#define _SDAP_API_H_

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Use to change the size of the service discovery cache.
 *
 * INPUT:
 *      U16 app_hdl: application hdl
 *      U32 cache_size: cache size
 *      U32 max_num_entrie:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *----------------------------------------------------------------------------*/
void sdap_set_cache_size_req(U16 app_hdl, U32 cache_size, U32 max_num_entry);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Use the discover svc (SD) module to search of other devices.
 *
 * INPUT:
 *      U16 app_hdl:
 *      U8 max_rsp:
 *      BOOL clear_filter:
 *      U16 timeout:
 *      U32 cls_dev:
 *      BOOL update_rmt_name:
 *      BOOL unknown_name:
 *      BOOL trusted:
 *
 *OUTPUT:
 *      void.
 *
 * NOTE:
 *     none.
 *----------------------------------------------------------------------------*/
void sdap_srch_req(U16 app_hdl,
                   U8 max_rsp,
                   BOOL clear_filter,
                   U16 timeout,
                   U32 dev_cls,
                   BOOL update_dev_disp_name,
                   BOOL unknow_name,
                   BOOL trusted);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Establish the search of other devices.
 *
 * INPUT:
 *      U16 app_hdl:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      NONE.
 *----------------------------------------------------------------------------*/
void sdap_esc_srch_req(U16 app_hdl);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Use the discover svc (SD) module to search of other device.
 *
 * INPUT:
 *      U16 app_hdl:
 *      U8 time_intvl:
 *      U32 dec_cls:
 *      BOOL update_rmt_name:
 *      BOOL unknown_name:
 *      BOOL trusted:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *     none.
 *----------------------------------------------------------------------------*/
void sdap_enb_srch_agent_req(U16 app_hdl,
                             U8 time_intvl,
                             U32 dev_cls,
                             BOOL update_dev_disp_name,
                             BOOL unknow_name,
                             BOOL trusted);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Disable the automatic search agent.
 *
 * INPUT:
 *      U16 app_hdl:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *----------------------------------------------------------------------------*/
void sdap_disb_srch_agent_req(U16 app_hdl);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Use the discovery service module to find the services offered by another devices.
 *
 * INPUT:
 *      U16 app_hdl:
 *      BTS2S_BD_ADDR bd:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *----------------------------------------------------------------------------*/
void sdap_rd_available_svc_req(U16 app_hdl, BTS2S_BD_ADDR bd);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Returns all information that the service discovery knows about a specific dev.
 *
 * INPUT:
 *      U16 app_hdl:
 *
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *----------------------------------------------------------------------------*/
void sdap_esc_rd_available_svc_req(U16 app_hdl);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Returns all information that the service discovery knows about a specific dev
 *
 * INPUT:
 *      U16 app_hdl:
 *      BTS2S_BD_ADDR bd:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      NONE.
 *----------------------------------------------------------------------------*/
void sdap_rd_dev_prop_req(U16 app_hdl, BTS2S_BD_ADDR bd);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Returns all information that the service discovery keeps in its cache
 *
 * INPUT:
 *      app_hdl:
 *
 * OUTPUT:
 *      void
 *
 * NOTE:
 *      NONE.
 *----------------------------------------------------------------------------*/
void sdap_rd_cache_req(U16 app_hdl);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Returns all information that the service discovery keeps in its cache.
 *
 * INPUT:
 *      U16 app_hdl
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      NONE.
 *----------------------------------------------------------------------------*/
void sdap_rd_trusted_dev_req(U16 app_hdl, U32 max_num_byte_res);


/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Returns all information that the SD holds about trusted devices.
 *
 * INPUT:
 *      U16 app_hdl:
 *      U32 max_num_bytes_in_res:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      NONE.
 *----------------------------------------------------------------------------*/
void sdap_rd_svcso_srch_for_req(U16 tid);

#ifdef __cplusplus
}
#endif

#endif

