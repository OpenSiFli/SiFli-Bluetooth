/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BTS2_APP_SYS_H_
#define _BTS2_APP_SYS_H_

#ifdef __cplusplus
extern "C" {
#endif

U32 bt_multibyte_to_widechar(const U32 CodePage,
                             const U32 dwFlags,
                             void     *lpMultiByteStr, //S8
                             U32       cbMultiByte,
                             void     *lpWideCharStr,  //S16
                             U32       cchWideChar);

U32 bt_widechar_to_multibyte(const U32 CodePage,
                             const U32 dwFlags,
                             void     *lpWideCharStr,      //LPCWSTR S16
                             U32       cchWideChar,
                             void     *lpMultiByteStr,     //S8 LPCSTR
                             U32       cbMultiByte,
                             void     *lpDefaultChar,      //S8 LPCSTR
                             void     *lpUsedDefaultChar); //LPBOOL


U32  bt_get_file_size(void *stream);

#ifdef __cplusplus
}
#endif

#endif


