/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_SBC_API_H_
#define _BTS2_SBC_API_H_

#include "bts2_type.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SBC_MAX_BLOCKS            16
#define SBC_MAX_CHNLS             2
#define SBC_MAX_SUBBANDS          8

typedef enum
{
    SBC_MONO = 0,
    SBC_DUAL,
    SBC_STEREO,
    SBC_JOINT_STEREO
} BTS2E_SBC_CHNL_MODE;

typedef enum
{
    SBC_METHOD_LOUDNESS = 0,
    SBC_METHOD_SNR
} BTS2E_SBC_ALLOC_METHOD;

typedef struct
{
    U8  *psrc;
    U16 src_len;
    U16 src_len_used;
    U8  *pdst;
    U16 dst_len;
    U16 dst_len_used;
} BTS2S_SBC_STREAM;

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Calculate the bit pool value before encoding or decoding.
 *
 *
 * INPUT:
 *      U8 *bit_pool_alt
 *      U8 *toggle_period
 *      BTS2E_SBC_CHNL_MODE chnl_mode
 *      U16 sample_freq
 *      U8 nrof_blocks
 *      U8 nrof_subbands
 *      U16 bitrate
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U8 bts2_sbc_calc_bit_pool(U8 *bit_pool_alt,
                          U8 *toggle_period,
                          BTS2E_SBC_CHNL_MODE chnl_mode,
                          U16 sample_freq,
                          U8 nrof_blocks,
                          U8 nrof_subbands,
                          U16 bitrate);

U16 bts2_sbc_calculate_framelen(BTS2E_SBC_CHNL_MODE chnl_mode, U8 nrof_blocks, U8 nrof_subbands, U8 bitpool);
U16 bts2_sbc_calculate_pcm_samples_per_sbc_frame(U8 nrof_blocks, U8 nrof_subbands);

/*----------------------------------------------------------------------------*
*
* DESCRIPTION:
*       Configure the settings required before encoding.
*
* INPUT:
*      BTS2E_SBC_CHNL_MODE chnl_mode
*      BTS2E_SBC_ALLOC_METHOD alloc_method
*      U16 sample_freq
*      U8 nrof_blocks
*      U8 nrof_subbands
*      U8 bitpool
*
* OUTPUT:
*      Frame size in bytes if inputs are valid else returns zero.
*
* NOTE:
*      none.
*
*----------------------------------------------------------------------------*/
U16 bts2_sbc_encode_cfg(BTS2E_SBC_CHNL_MODE chnl_mode,
                        BTS2E_SBC_ALLOC_METHOD alloc_method,
                        U16 sample_freq,
                        U8 nrof_blocks,
                        U8 nrof_subbands,
                        U8 bitpool);

/*----------------------------------------------------------------------------*
*
* DESCRIPTION:
*       Configure the settings required before decoding.
*
* INPUT:
*      BTS2E_SBC_CHNL_MODE chnl_mode
*      BTS2E_SBC_ALLOC_METHOD alloc_method
*      U16 sample_freq
*      U8 nrof_blocks
*      U8 nrof_subbands
*      U8 bitpool
*
* OUTPUT:
*      If inputs are valid else returns FALSE.
*
* NOTE:
*      none.
*
*----------------------------------------------------------------------------*/
U8 bts2_sbc_decode_cfg(BTS2E_SBC_CHNL_MODE chnl_mode,
                       BTS2E_SBC_ALLOC_METHOD alloc_method,
                       U16 sample_freq,
                       U8 nrof_blocks,
                       U8 nrof_subbands,
                       U8 bitpool);

/*----------------------------------------------------------------------------*
*
* DESCRIPTION:
*      SBC encode.
*
*
*
* INPUT:
*      BTS2S_SBC_STREAM *pbss
*
*
* OUTPUT:
*      void.
*
* NOTE:
*      none.
*
*----------------------------------------------------------------------------*/
void bts2_sbc_encode(BTS2S_SBC_STREAM *pbss);

/*----------------------------------------------------------------------------*
*
* DESCRIPTION:
*      SBC decode.
*
*
*
* INPUT:
*      BTS2S_SBC_STREAM *pbss
*
*
* OUTPUT:
*      void.
*
* NOTE:
*      none.
*
*----------------------------------------------------------------------------*/
void bts2_sbc_decode(BTS2S_SBC_STREAM *pbss);

void bts2_sbc_encode_completed(void);


void bts2_sbc_decode_completed(void);


///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*----------------------------------------------------------------------------*
*
* DESCRIPTION:
*       Configure the settings required before encoding.
*
* INPUT:
*
* OUTPUT:
*      Frame size in bytes if inputs are valid else returns zero.
*
* NOTE:
*      none.
*
*----------------------------------------------------------------------------*/
U16 bts2_msbc_encode_cfg(void);

/*----------------------------------------------------------------------------*
*
* DESCRIPTION:
*       Configure the settings required before decoding.
*
* INPUT:
*
* OUTPUT:
*      If inputs are valid else returns FALSE.
*
* NOTE:
*      none.
*
*----------------------------------------------------------------------------*/
U8 bts2_msbc_decode_cfg(void);

/*----------------------------------------------------------------------------*
*
* DESCRIPTION:
*      mSBC encode.
*
*
*
* INPUT:
*      BS_SBC_STREAM *pbss
*
*
* OUTPUT:
*      void.
*
* NOTE:
*      none.
*
*----------------------------------------------------------------------------*/
void bts2_msbc_encode(BTS2S_SBC_STREAM *pbss);

/*----------------------------------------------------------------------------*
*
* DESCRIPTION:
*      mSBC decode.
*
*
*
* INPUT:
*      BS_SBC_STREAM *pbss
*
*
* OUTPUT:
*      void.
*
* NOTE:
*      none.
*
*----------------------------------------------------------------------------*/
void bts2_msbc_decode(BTS2S_SBC_STREAM *pbss);


void bts2_msbc_encode_completed(void);


void bts2_msbc_decode_completed(void);


#ifdef __cplusplus
}
#endif

#endif

