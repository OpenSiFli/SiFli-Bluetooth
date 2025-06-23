/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_USB_SYS_H_
#define _BTS2_USB_SYS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*buff types */
typedef struct
{
    char     *buf;
    unsigned buflen;
} BTS2S_HCI_TX_BUF;

typedef struct
{
    char     type;
    char     *buf;
    unsigned buflen;
} BTS2S_HCI_RX_BUF;

#ifndef BTS2S_RECV_MSG_DEF
#define BTS2S_RECV_MSG_DEF
typedef struct
{
    char *buf;
    unsigned buflen;
    unsigned dex;
} BTS2S_RECV_DATA;
#endif

BOOL usb_drv_start(char *name);
void usb_drv_stop(void);
void usb_drv_reset(void);
void  bts2d_usb_rx(void);

BOOL bts2d_usb_tx(U8 chnl, U8 *data, U16 size);


#ifdef __cplusplus
}
#endif

#endif

