/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_MSG_H_
#define _BTS2_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BTS2M_RESET          0x007F
#define BTS2MD_START         0x0000
#define BTS2MU_START         0x8000

//bug2018 temp test 10->20
//#define BTS2_FIFO_MSG_NUM   10
#define BTS2_FIFO_MSG_NUM   20

#define MAX_TIME            2000
#define INFINITY_TIME       0

typedef struct
{
    U16 ev;
    void *msg;
} BTS2S_MSG_STORE_FIFO_ELEM_TYPE;

typedef struct BTS2S_MSG_STORE_FIFO_TYPE_TAG
{
    U16 msg_num;
    U16 next_num;
    struct BTS2S_MSG_STORE_FIFO_TYPE_TAG  *next;
    BTS2S_MSG_STORE_FIFO_ELEM_TYPE fifo[BTS2_FIFO_MSG_NUM];
} BTS2S_MSG_STORE_FIFO_TYPE;

typedef void (ev_fn_type)(U16, void *);
typedef struct BTS2S_TIME_EV_TYPE_TAG
{
    struct BTS2S_TIME_EV_TYPE_TAG *next;
    U32        time;
    ev_fn_type *ev_fn;
    U16        ip;
    void       *vp;
    TID        id;
} BTS2S_TIME_EV_TYPE;

typedef struct
{
    U16    ev;
    U16    task;
    TID    id;
    void   *msg;
} BTS2S_TIME_MSG_TYPE;

typedef struct BTS2S_MSG_FIFO_TYPE_TAG
{
    struct BTS2S_MSG_FIFO_TYPE_TAG *next;
    struct BTS2S_MSG_FIFO_TYPE_TAG *prev;
    void       *msg;
    U32        id;
    U16        ev;
} BTS2S_MSG_FIFO_TYPE;

typedef enum
{
    BTS2M_HCI_EVT = 1,
    BTS2M_HCI_CMD,
    BTS2M_L2C,
    BTS2M_RFC,
    BTS2M_SDP,
    BTS2M_GAP,
    BTS2M_SC,
    BTS2M_SDAP,
    BTS2M_SPP_CLT,
    BTS2M_SPP_SRV,
    BTS2M_HFP_AG,
    BTS2M_HFP_HF,
    BTS2M_OPP_SRV,
    BTS2M_OPP_CLT,
    BTS2M_FTP_SRV,
    BTS2M_FTP_CLT,
    BTS2M_BIP_CLT,
    BTS2M_BIP_SRV,
    BTS2M_BPP_CLT,
    BTS2M_BPP_SRV,
    BTS2M_AV,
    BTS2M_AVRCP,
    BTS2M_HID,
    BTS2M_HID_HOST,
    BTS2M_DUN_CLT,
    BTS2M_DUN_SRV,
    BTS2M_FAX_CLT,
    BTS2M_FAX_SRV,
    BTS2M_BNEP,
    BTS2M_PAN,
    BTS2M_HCRP_SRV,
    BTS2M_SYNC_SRV,
    BTS2M_SAP_CLT,
    BTS2M_SAP_SRV,
    BTS2M_PBAP_CLT,
    BTS2M_PBAP_SRV,
    BTS2M_BR_GATT,
    BTS2M_BLE_CM,
    BTS2M_BT_DIS,
    BTS2M_BT_L2CAP_PROFILE
} BTS2_MSG;

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Send one message to between different tasks.
 *
 * INPUT:
 *      U16 task: task id.
 *      U16 ip: message type.
 *      void *vp: pointer of message.
 *
 * OUTPUT:
 *      message id.
 *
 * NOTE:
 *      void.
 *
 *----------------------------------------------------------------------------*/
U32 bts2_msg_put(U16 task, U16 ip, void *vp);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Get one message from the specified task.
 *
 * INPUT:
 *      U16 task: task id.
 *      U16 *pi: pointer of message type.
 *      void **pv: pointer of pointer of message.
 *
 * OUTPUT:
 *      TRUE if successful, otherwise FALSE.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
BOOL bts2_msg_get(U16 task, U16 *pi, void **pv);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Put one message to task after delay.
 *
 * INPUT:
 *      U32 delay_msel: time interval in milliseconds.
 *      U16 task: task id.
 *      U16 ip: message type.
 *      void *vp: pointer of message.
 *
 * OUTPUT:
 *      Timer event id will returned.
 *
 * NOTE:
 *      delay_msel must be in milliseconds.
 *
 *----------------------------------------------------------------------------*/
U32 bts2_timer_msg_add(U32 delay_msel, U16 task, U16 ip, void *vp);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Put the timed message.
 *
 * INPUT:
 *      void *v: pointer of timed message.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      void.
 *
 *----------------------------------------------------------------------------*/
void bts2_timer_msg_put(U16 pp, void *v);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Cancel one timed message put.
 * INPUT:
 *      U16 task: task id.
 *      U32 id: message unique id.
 *      U16 *pi: pointer of message type.
 *      void **pv: pointer of pointer of message.
 *
 * OUTPUT:
 *      TRUE if successful, otherwise FALSE.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
BOOL bts2_timed_msg_cancel(U16 task, U32 id, U16 *pi, void **pv);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Add one timer with call back function which should be called at TIME.
 *
 * INPUT:
 *      U32 time: system time.
 *      void (*fn) (U16, void *): event call back function.
 *      U16 ip: message type.
 *      void *vp: pointer of message.
 *
 * OUTPUT:
 *      Timer event id will returned.
 *
 * NOTE:
 *      void.
 *
 *----------------------------------------------------------------------------*/
U32 bts2_timer_ev_time(U32 time, void (* fn)(U16, void *), U16 ip, void *vp);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Add one timer with call back function which should be called after delay.
 *
 * INPUT:
 *      U32 delay_msel: time interval in milliseconds.
 *      U16 task: task id.
 *      U16 ip: message type.
 *      void *vp: pointer of message.
 *
 * OUTPUT:
 *      Timer event id will returned.
 *
 * NOTE:
 *      delay_msel must be in milliseconds.
 *
 *----------------------------------------------------------------------------*/
U32 bts2_timer_ev_add(U32 delay_msel, void (*fn)(U16, void *), U16 ip, void *vp);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Cancel timed event.
 *      occurring.
 *
 * INPUT:
 *      TID id: timer id.
 *      U16 *pi: pointer of message type.
 *      void **pv: pointer of pointer of message.
 *
 * OUTPUT:
 *      TRUE if successful, otherwise FALSE.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
BOOL bts2_timer_ev_cancel(TID id, U16 *pi, void **pv);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function will push one message to one specified fifo.
 *
 * INPUT:
 *      BTS2S_MSG_STORE_FIFO_TYPE **fifo:
 *      U16 ev:
 *      void *msg:
 *
 * OUTPUT:
 *      none.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void bts2_store_msg_push(BTS2S_MSG_STORE_FIFO_TYPE **fifo, U16 ev, void *msg);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function will pop the first pushed message from fifo out.
 *
 * INPUT:
 *      BTS2S_MSG_STORE_FIFO_TYPE **fifo:
 *      U16 *ev:
 *      void **msg:
 *
 * OUTPUT:
 *      TRUE if successful, otherwise FALSE.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
BOOL bts2_store_msg_pop(BTS2S_MSG_STORE_FIFO_TYPE **fifo, U16 *ev, void **msg);

#ifdef __cplusplus
}
#endif

#endif

