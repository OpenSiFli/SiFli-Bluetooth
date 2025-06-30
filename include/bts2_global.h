/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_GLOBAL_H_
#define _BTS2_GLOBAL_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "rtthread.h"
#include "config.h"
#include "string.h"
#include <stdint.h>
#include "stdlib.h"
#include "bts2_type.h"
#include "bts2_bt.h"
#include "bts2_util.h"
#include "bts2_buff.h"
#include "bts2_fsm.h"
#include "bts2_mem.h"
#include "bts2_sys.h"
#include "bts2_msg.h"
#include "bts2_dbg.h"
#include "bts2_api.h"
#include "bts2_file.h"

/* temp put here */
#if defined(CFG_GNU) || defined(CFG_ANDROID)
    #pragma   pack  (4)
#endif

#define CFG_SBC

#ifdef __cplusplus
}
#endif


#endif

