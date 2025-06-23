/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_MEM_H__
#define _BTS2_MEM_H__
#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*
 *
 * DESCRIPTION:
 *      The bmemmove function copies count bytes of characters from src to dest.
 *      If some regions of the source area and the destination overlap, bmemmove
 *      ensures that the original source bytes in the overlapping region are
 *      copied before being overwritten.
 *
 * INPUT:
 *      void *dest: Destination object.
 *      const void *src: Source object.
 *      int count: Number of characters to copy.
 *
 * OUTPUT:
 *      bmemcpy returns the value of dest.
 *
 * NOTE:
 *      none.
 *
 *----------------------------------------------------------------------------*/
void *bmemmove(void *dest, const void *src, U32 count);

void *bt_mem_alloc(size_t size);
void *bt_mem_calloc(size_t count, size_t nbytes);
void bt_mem_free(void *ptr);


//void *bmalloc(U32 nbytes);
#define bmalloc(size) bt_mem_alloc(size)
#define bcalloc(count, nbytes) bt_mem_calloc(count, nbytes)
#define bfree(ptr) bt_mem_free(ptr)
//void *bcalloc(rt_size_t count, rt_size_t nbytes);
//void bfree(void *ptr);
#ifndef malloc
#define malloc bmalloc
#endif

#ifndef free
#define free bfree
#endif

#ifndef calloc
#define calloc(unit,size) bcalloc(unit, size)
#endif
#ifdef __cplusplus
}
#endif
#endif

