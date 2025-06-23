/*
 * SPDX-FileCopyrightText: 2019-2025 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bts2_app_inc.h"

#ifdef CFG_MS
    #include "windows.h"
#endif

U32 bt_multibyte_to_widechar(const U32 CodePage,
                             const U32 dwFlags,
                             void     *lpMultiByteStr, //S8
                             U32       cbMultiByte,
                             void     *lpWideCharStr,  //S16
                             U32       cchWideChar)
{
#ifdef CFG_MS
    return MultiByteToWideChar(CodePage,
                               dwFlags,
                               (LPCSTR)lpMultiByteStr,
                               cbMultiByte,
                               (LPWSTR)lpWideCharStr,
                               cchWideChar);
#endif
    return 0;
}

U32 bt_widechar_to_multibyte(const U32 CodePage,
                             const U32 dwFlags,
                             void     *lpWideCharStr, //LPCWSTR S16
                             U32       cchWideChar,
                             void     *lpMultiByteStr,//S8 LPCSTR
                             U32       cbMultiByte,
                             void     *lpDefaultChar, //S8 LPCSTR
                             void     *lpUsedDefaultChar)
{
#ifdef CFG_MS
    return  WideCharToMultiByte(CodePage,
                                dwFlags,
                                (LPCWSTR)lpWideCharStr,
                                cchWideChar,
                                (LPSTR) lpMultiByteStr,
                                cbMultiByte,
                                (LPCSTR)lpDefaultChar,
                                (LPBOOL)lpUsedDefaultChar);
#endif
    return 0;
}

U32 bt_get_file_size(void *stream)
{
    U32 curpos;
    U32 length = 0;

    curpos = ftell((FILE *)stream);
    fseek((FILE *)stream, 0L, SEEK_END);
    length = ftell((FILE *)stream);
    fseek((FILE *)stream, curpos, SEEK_SET);

    return length;
}

