/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_TYPE_H_
#define _BTS2_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#undef  FALSE
#define FALSE   (0)

#undef  TRUE
#define TRUE    (1)

#ifndef NULL
#define NULL    (0)
#endif

#define EPHDL                   0xFFFF /* Error handle */

#define CFG_ARM


#if defined(CFG_MS) || defined(CFG_WINCE) || defined(CFG_ARM)

#define CP_ACP                    0           // default to ANSI code page
#define CP_OEMCP                  1           // default to OEM  code page
#define CP_MACCP                  2           // default to MAC  code page
#define CP_THREAD_ACP             3           // current thread's ANSI code page
#define CP_SYMBOL                 42          // SYMBOL translations

#define CP_UTF7                   65000       // UTF-7 translation
#define CP_UTF8                   65001       // UTF-8 translation

typedef unsigned char           U8;
typedef unsigned short int      U16;
typedef unsigned long int       U24;
typedef unsigned long int       U32;

typedef signed char             S8;
typedef signed short int        S16;
typedef signed long int         S24;
typedef signed long int         S32;
typedef unsigned short          WORD;
typedef void                   *LPVOID;
//typedef wchar_t               WCHAR;

#ifndef BOOL
typedef int BOOL;
#endif
#endif

#if defined(CFG_GNU) || defined(CFG_ANDROID)

#define CP_ACP                    0           // default to ANSI code page
#define CP_OEMCP                  1           // default to OEM  code page
#define CP_MACCP                  2           // default to MAC  code page
#define CP_THREAD_ACP             3           // current thread's ANSI code page
#define CP_SYMBOL                 42          // SYMBOL translations

#define CP_UTF7                   65000       // UTF-7 translation
#define CP_UTF8                   65001       // UTF-8 translation


typedef unsigned char           U8;
typedef unsigned short int      U16;
typedef unsigned long int       U24;
typedef unsigned long int       U32;

typedef signed char             S8;
typedef signed short int        S16;
typedef signed long int         S24;
typedef signed long int         S32;
typedef unsigned short          WORD;
typedef void                   *LPVOID;
typedef unsigned short          WCHAR;

#ifndef BOOL
typedef int BOOL;
#endif
#endif


/* depend on BTS2_GET_TIME */
#define MILLISECOND             ((U32)(1))
#define SECOND                  ((U32)(1000 * MILLISECOND))
#define MINUTE                  ((U32)(60 * SECOND))
typedef unsigned long int       TID;

#ifdef __cplusplus
}
#endif

#endif

