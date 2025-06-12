/**
  ******************************************************************************
  * @file   a2dp_relay.h
  * @author Sifli software development team
  * @brief A2DP relay service internal header file.
 *
  ******************************************************************************
*/
/**
 * @attention
 * Copyright (c) 2025 - 2025,  Sifli Technology
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Sifli integrated circuit
 *    in a product or a software update for such product, must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Sifli nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Sifli integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY SIFLI TECHNOLOGY "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SIFLI TECHNOLOGY OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifndef A2DP_RELAY_H_
#define A2DP_RELAY_H_

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <stdlib.h>
#include "bf0_sibles.h"

#define A2DP_RELAY_ID 0x26
#define A2DP_RELAY_INVALID_HDL 0xFF
#define RELAY_INTERVAL  (20)

#define A2DP_RELAY_SVC_UUID { \
    0x73, 0x69, 0x66, 0x6c, \
    0x69, 0x5f, 0x72, 0x26, \
    0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00 \
}

/* media codecs */
#define CODEC_SBC    ((U8)0x00)
#define CODEC_AAC    ((U8)0x02)

#define  PLAYBACK_GETDATA_EVENT_FLAG       (1 << 0)
#define  PLAYBACK_START_EVENT_FLAG         (1 << 1)
#define  PLAYBACK_STOPPING_EVENT_FLAG      (1 << 2)
#define  PLAYBACK_STOPPED_EVENT_FLAG       (1 << 3)

#define DEPLAYBACK_STACK_SIZE   2048

#define A2DP_RELAY_SRC_SEARCH_DURATION (60) // 60s
#define A2DP_RELAY_SRC_SEARCH_INTERVAL (60) // 60ms
#define A2DP_RELAY_SRC_SEARCH_WINDOW   (30) // 30ms

typedef enum
{
    A2DP_RELAY_START,
    A2DP_RELAY_DATA,
    A2DP_RELAY_STOP,
    A2DP_RELAY_VOL,
} A2DP_RELAY_TYPE_t;

typedef enum
{
    A2DP_RELAY_NO_ROLE,
    A2DP_RELAY_SINK,
    A2DP_RELAY_SRC,
} A2DP_RELAY_ROLE_t;

typedef enum
{
    A2DP_RELAY_ERR_NO_ERROR,
    A2DP_RELAY_ERR_GENERAL,
    A2DP_RELAY_ERR_ALREADY_CONN,
    A2DP_RELAY_ERR_UNDER_RUN,
    A2DP_RELAY_ERR_INVALID,
} A2DP_RELAY_ERR_t;

typedef enum
{
    A2DP_RELAY_EVT_ENABLED,
    A2DP_RELAY_EVT_PAIRED,
    A2DP_RELAY_EVT_DISABLED,
} A2DP_RELAY_EVT_t;

typedef struct
{
    uint8_t blocks;
    uint8_t subbands;
    uint8_t chnl_mode;
    uint8_t alloc_method;
    uint8_t bitpool;
    uint16_t sample_freq;
} a2dp_relay_sbc_para_t;

typedef struct
{
    uint8_t type;
    uint16_t len;
    uint8_t data[__ARRAY_EMPTY];
} a2dp_relay_data_t;

typedef struct
{
    uint8_t blocks;
    uint8_t frms_per_payload;
    uint8_t subbands;
    uint8_t codec;
    uint16_t sample_freq;
} a2dp_relay_start_para_t;

typedef uint8_t (*a2dp_relay_callback)(A2DP_RELAY_EVT_t evt, uint8_t *data, uint16_t len);

// A2DP RELAY SRC APIs
void a2dp_relay_src_init(a2dp_relay_callback callback);
uint8_t a2dp_relay_src_enable(void);
uint8_t a2dp_relay_src_disable(void);
uint8_t a2dp_relay_vol_set(uint8_t vol);
uint8_t a2dp_relay_src_channel_set(uint8_t channel);

// A2DP RELAY SINK APIs
void a2dp_relay_sink_init(a2dp_relay_callback callback);
uint8_t a2dp_relay_sink_disable(void);
uint8_t a2dp_relay_sink_enable(void);

#endif // A2DP_RELAY_H_

/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/