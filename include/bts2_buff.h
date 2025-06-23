/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_BUFF_H_
#define _BTS2_BUFF_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BTS2S_BUFF_POOL_TAG
{
    U16 wr_hdl;
    U16 rd_hdl;
    U32 size;
    U8  *mem;
} BTS2S_BUFF_POOL;

typedef struct BTS2S_BUFF_PATCH_TAG
{
    BTS2S_BUFF_POOL *pool;
    U32            start;
    U32            len;
    BOOL           bfirst;
} BTS2S_BUFF_PATCH;

typedef struct BTS2S_BUFF_TAG
{
    U32         offset;
    U32         len;
    U8          bnep_network;
    U8          from_bnep_clt;
    U8          bnep_network_special; /* if it is bnep_network,does first word only contain an octet? */
    U8          *data;
    void        *rel;
    U8          *data_orig;
    BOOL        use_offset;
    U16         len_orig;

    BTS2S_BUFF_PATCH *patch;
    struct BTS2S_BUFF_TAG *prev;
    struct BTS2S_BUFF_TAG *next;
    U8          flag;
} BTS2S_BUFF;

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Initialize the BUFF module.
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      void
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void buff_init(void);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Create a new BUFF with the given size.
 *
 * INPUT:
 *      U32 size.
 * OUTPUT:
 *      Pointer of the new BUFF or NULL if failed.
 *
 * NOTE:
 *      none.
 *
*----------------------------------------------------------------------------*/
BTS2S_BUFF *buff_malloc(U32 size);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Free the input BUFF list.
 *
 * INPUT:
 *      BTS2S_BUFF *buff:
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void buff_free(BTS2S_BUFF *buff);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Read the specified length of data from the LIST to BUFF.
 *
 * INPUT:
 *      BTS2S_BUFF **list
 *      U8        *buff
 *      U32       size
 *
 * OUTPUT:
 *      the length of red data.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U32 buff_read(BTS2S_BUFF **list,
              U8 *buff,
              U32 size);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Read the specified length of data from LIST to BUFF with starting
 *      from position OFFSET bytes from the start of list.
 *
 * INPUT:
 *      const BTS2S_BUFF *list,
 *      U32             offset
 *      U8              *buff
 *      U32             size
 * OUTPUT:
 *      the length of red data.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U32 buff_get(const BTS2S_BUFF *list,
             U32 offset,
             U8 *buff,
             U32 size);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      ADD the new BUFF to the end of LIST.
 *
 * INPUT:
 *      BTS2S_BUFF **list
 *      BTS2S_BUFF *buff
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
*
*----------------------------------------------------------------------------*/
void buff_add(BTS2S_BUFF **list, BTS2S_BUFF *buff);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Get the pointer of BUFF data with OFFSET.
 *
 * INPUT:
 *      const BTS2S_BUFF *buff
 *
 * OUTPUT:
 *      U8 *: pointer of data with offset.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U8 *buff_map(const BTS2S_BUFF *buff);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Copy one BUFF to another new allocated one.
 *
 * INPUT:
 *      const BTS2S_BUFF *buff:
 *
 * OUTPUT:
 *      Pointer of the new BUFF.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
BTS2S_BUFF *buff_copy(const BTS2S_BUFF *buff);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Find the related BUFF handle with the specified connection handle.
 *
 * INPUT:
 *      U16 conn_hdl
 *
 * OUTPUT:
 *      BUFF handle if found, otherwise NULL.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
BTS2S_BUFF_POOL *buff_get_hdl(U16 conn_hdl);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Get the total length of a BUFF list.
 *
 * INPUT:
 *      BTS2S_BUFF *list
 *
 * OUTPUT:
 *      the length of the BUFF list.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U32 buff_get_len(BTS2S_BUFF *list);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      Set the read position of LIST.
 *
 * INPUT:
 *      BTS2S_BUFF **list
 *      U16 pos
 *
 * OUTPUT:
 *      byte number
 *
 * NOTE:
 *      none.
 *
*----------------------------------------------------------------------------*/
U32 buff_set_rd_pos(BTS2S_BUFF **list, U32 pos);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      BUFF patch to data stream.
 *
 * INPUT:
 *      BTS2S_BUFF *buff
 *      BOOL flag
 *
 * OUTPUT:
 *      data pointer.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
U8 *buff_patch_2_stream(BTS2S_BUFF *buff, BOOL flag);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      BUFF to data stream.
 *
 * INPUT:
 *      BTS2S_BUFF *list
 *
 * OUTPUT:
 *      void.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void buff_convert_patch(BTS2S_BUFF *list);

#ifdef CFG_PAN
/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
BTS2S_BUFF *buff_new_in(U16 data_len, BOOL special);

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *
 *
 * INPUT:
 *
 * OUTPUT:
 *
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
BTS2S_BUFF *buff_insert(BTS2S_BUFF *p_first, BTS2S_BUFF *p_rest);

#endif

#ifdef __cplusplus
}
#endif

#endif

