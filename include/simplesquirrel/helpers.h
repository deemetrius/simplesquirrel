#ifndef SSQ_HELPERS_HEADER_H
#define SSQ_HELPERS_HEADER_H

#pragma once

#include <climits>
#include <string>

//
#include "utf_impl.h"


#ifdef SQUNICODE
    #include <wchar.h>
    #include <wctype.h>
    
    #ifdef _MSC_VER
    #define scvprintf   vwprintf
    #define scvfprintf  vfwprintf
    #else
    #define scvprintf   vwprintf
    #define scvfprintf  vfwprintf
    #endif
#else
    #include <stdio.h>

    #ifdef _MSC_VER
    #define scvprintf   vprintf
    #define scvfprintf  vfprintf
    #else
    #define scvprintf   vprintf
    #define scvfprintf  vfprintf
    #endif
#endif


namespace ssq {


inline
std::string ToUtf8(const std::wstring &str)
{

    #if WCHAR_MAX <= 0xFFFFu /* wchar_t is 16 bit width, signed or unsigned */

        auto  utfCharsStr32        = utf32_from_utf16(str);
        const utf32_char_t *pBegin = utfCharsStr32.data();
        return string_from_utf32(pBegin, pBegin+utfCharsStr32.size());

    #else

        const utf32_char_t *pBegin = str.data();
        return string_from_utf32(pBegin, pBegin+str.size());

    #endif

    // auto pApi = getEncodingsApi();
    // return pApi->encode(str, EncodingsApi::cpid_UTF8);
}

inline
std::string ToUtf8(const wchar_t *pStr)
{
    if (!pStr)
    {
        return std::string();
    }

    return ToUtf8(std::wstring(pStr));
}

inline
std::string ToUtf8(const std::string &str)
{
    return str;
}

inline
std::string ToUtf8(const char *pStr)
{
    if (!pStr)
    {
        return std::string();
    }

    return ToUtf8(std::string(pStr));
}


} // namespace ssq {


#endif /* SSQ_HELPERS_HEADER_H */