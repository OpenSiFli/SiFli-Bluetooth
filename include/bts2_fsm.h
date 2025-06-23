/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_FSM_H_
#define _BTS2_FSM_H_

/* event value used to exit from FSM */
#define BTS2_FSM_EV_FINISH           0xFF    /* max event number should within 255 */
#define BTS2_FSM_IDX_NULL            0xFF    /* max function index should within 255 */

/* result code of handler function */
#define BTS2_FSM_FAIL                0
#define BTS2_FSM_SUCC                1

/* prototype of handler function of FSM,  */
typedef U8(*BTS2_FSM_FUNC)(void *p1, void *p2, U8 *pst);

/* BTS2 FSM Instance definition */
typedef struct
{
    const BTS2_FSM_FUNC *func;
    const U8 *matrix;
#ifdef CONFIG_DBG
    const U8 *fsm_name;   /* string of FSM name */
#endif
    const U8 max_st;
    const U8 max_ev;
} BTS2S_FSM_INST;

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *     The engine function for BTS2 full State Machine which will keep running
 *     until until there are no more events.
 *
 * INPUT:
 *     const BTS2S_FSM_INST *inst
 *     U8 *pst
 *     U8 ev
 *     void *p1
 *     void *p2
 *
 * OUTPUT:
 *     BTS2_FSM_SUCC if successful, otherwise BTS2_FSM_FAIL.
 *
 * NOTE:
 *     the instant function should have *pst = new_state code sentence.
 *----------------------------------------------------------------------------*/
U8 bts2_fsm(const BTS2S_FSM_INST *inst, U8 *pst, U8 ev, void *p1, void *p2);

#endif

