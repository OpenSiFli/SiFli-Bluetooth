/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_API_H_
#define _BTS2_API_H_
#ifdef __cplusplus
extern "C" {
#endif
#define BTS2_VER_LEN     25

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      BTS2 initialize related resources to prepare to run.
 *
 * INPUT:
 *      BTS2S_DB_FILE_PATH path: DB file path.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      True if successful, otherwise failed.
 *
 *----------------------------------------------------------------------------*/
U8 bts2_init(BTS2S_DB_FILE_PATH path);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Start the BTS2 task message loop running.
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
void bts2_run(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Let the BTS2 task message loop begin to run each task release function
 *      to release related resources.
 *
 * INPUT:
 *      void.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      True if successful, otherwise failed.
 *
 *----------------------------------------------------------------------------*/
BOOL bts2_rel(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Let the BTS2 task message loop running exit.
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
void bts2_stop(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Get the BTS2 version.
 *
 * INPUT:
 *      char *ver: The char string used to get the BTS2 version back.
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      The input string length should not be less than BTS2_VER_LEN.
 *
 *----------------------------------------------------------------------------*/
void bts2_get_ver(char *ver);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *     This function used to get app task id.
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
U16 bts2_task_get_app_task_id();

U16 bts2_task_get_pan_task_id();



U16 bts2_get_l2cap_default_mtu_size(void);

#ifdef __cplusplus
}
#endif

#endif

