/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_SYS_H_
#define _BTS2_SYS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BTS2_CRITICAL_TASK               0
#define BTS2_CRITICAL_INTERRUPT          (BTS2_CRITICAL_TASK + 1)
#ifdef CFG_BTS2_PROTECT
#define BTS2_CRITICAL_TASK_PROTECT       (BTS2_CRITICAL_INTERRUPT + 1)
#else
#define BTS2_CRITICAL_TASK_PROTECT       BTS2_CRITICAL_INTERRUPT
#endif
#define BTS2_CRITICAL_GET_TSK_ID         (BTS2_CRITICAL_TASK_PROTECT + 1)
#define BTS2_CRITICAL_NUM                (BTS2_CRITICAL_GET_TSK_ID + 1)

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function initializes a critical section object. The purpose of
 *      design is provide critical sections are used to ensure that certain
 *      sections of code are thread-safe or interrupt-safe, depending on the
 *      nature of the underlying operating system.
 *
 * INPUT:
 *      U8 id:  BTS2_CRITICAL_TASK
 *              BTS2_CRITICAL_INTERRUPT
 *              BTS2_CRITICAL_TASK_PROTECT
 *              BTS2_CRITICAL_GET_TSK_ID
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void BTS2_CREATE_CRITICAL(U8 id);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function waits for ownership of the specified critical section
 *      object. It returns when the calling Task is granted ownership.
 *
 * INPUT:
 *      U8 id:  BTS2_CRITICAL_TASK
 *              BTS2_CRITICAL_INTERRUPT
 *              BTS2_CRITICAL_TASK_PROTECT
 *              BTS2_CRITICAL_GET_TSK_ID
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void BTS2_ENTER_CRITICAL(U8 id);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function releases ownership of the specified critical section object.
 *
 * INPUT:
 *      U8 id:  BTS2_CRITICAL_TASK
 *              BTS2_CRITICAL_INTERRUPT
 *              BTS2_CRITICAL_TASK_PROTECT
 *              BTS2_CRITICAL_GET_TSK_ID
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void BTS2_LEAVE_CRITICAL(U8 id);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function releases all resources used by an unowned critical section
 *      object required.
 *
 * INPUT:
 *      U8 id:  BTS2_CRITICAL_TASK
 *              BTS2_CRITICAL_INTERRUPT
 *              BTS2_CRITICAL_TASK_PROTECT
 *              BTS2_CRITICAL_GET_TSK_ID
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void BTS2_DELETE_CRITICAL(U8 id);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *     This function creates the EVENT object which used to wake up Task message
 *     loop from sleeping status.
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
void BTS2_CREATE_EVENT(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function sets the state of the wake up event object to signaled.
 *      Used to interrupt the Task message loop from sleeping.
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
void BTS2_SET_EVENT(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function returns when the specified object is in the signaled state
 *      or when the time-out interval elapses.
 *
 * INPUT:
 *      U32 milliseconds.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void BTS2_WAIT_4_MSEL_EVENT(U32 milliseconds);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function returns when the specified object is in the signaled state.
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
void BTS2_WAIT_4_EVENT(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function deletes the opened wake up EVENT of Task message loop.
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
void BTS2_DELETE_EVENT(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      This function retrieves the number of milliseconds that have elapsed
 *      since system start.
 *
 * INPUT:
 *      void.
 *
 * OUTPUT:
 *      The number of milliseconds.
 *
 * NOTE:
 *      The roughly precision of milliseconds number output must be meet on
 *      the target system.
 *
 *----------------------------------------------------------------------------*/
U32 BTS2_GET_TIME(void);
#ifdef __cplusplus
}
#endif
#endif


