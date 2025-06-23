/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _HCI_DRV_H_
#define _HCI_DRV_H_

#include "bts2_type.h"
/* this struct be defined in driver */
#ifndef BTS2S_RECV_MSG_DEF
#define BTS2S_RECV_MSG_DEF
typedef struct
{
    char *buf;
    unsigned buflen;
    unsigned dex;
} BTS2S_RECV_DATA;
#endif

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      void
 *
 * NOTE:
 *      none.
 *----------------------------------------------------------------------------*/

void hcid_reset(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      received a HCI cmd and send it.
 *
 * INPUT:
 *      U8 *msg
 *      U16 len
 *
 * OUTPUT:
 *      void
 *
 * NOTE:
 *      none.
 *----------------------------------------------------------------------------*/

BOOL hcid_send_hci_cmd(U8 *cmd, U16 len);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      received an acl data pkt and send it.
 *
 * INPUT:
 *      U8 *msg
 *      U16 len
 *
 * OUTPUT:
 *      void
 *
 * NOTE:
 *      none.
 *----------------------------------------------------------------------------*/

BOOL hcid_send_acl_data(U8 *data, U16 len);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      received a sco data pkt and send it.
 *
 * INPUT:
 *      U8 *msg
 *      U16 len
 *
 * OUTPUT:
 *      void
 *
 * NOTE:
 *      none.
 *----------------------------------------------------------------------------*/

BOOL hcid_send_sco_data(U8 *data, U16 len);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function used to send HCI vendor command out.
 *
 * INPUT:
 *      U8 *cmd
 *      U16 len
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void hcid_send_vendor_cmd(U8 *cmd, U16 len);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *    U8 ch
 *    U8 type
 *    U8 *msg
 *    U16 len
 *
 * OUTPUT:
 *    void.
 *
 * NOTE:
 *    hcid_bcsp_send(BCSP_CHNL_HQ, BCSP_RELIABLE_CHNL, msg, len) can be used
 *    to send HQ command.
 *
 *----------------------------------------------------------------------------*/
void hcid_bcsp_send(U8 ch, U8 type, U8 *data, U16 len);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      The function used to handle all type of data from controller.
 *
 * INPUT:
 *      void *msg
 *      U16 chnl
 *      U16 rel
 * OUTPUT:
 *      void
 *
 * NOTE:
 *      none.
 *----------------------------------------------------------------------------*/

void hcid_recv_data(void *msg, U8 chnl, U16 rel);
#endif

