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


#ifdef SQUNICODE

    typedef std::wstring  sqstring;

    template<typename T>
    sqstring to_sqstring(T t)
    {
        return std::to_wstring(t);
    }

#else

    typedef std::string   sqstring;

    template<typename T>
    sqstring to_sqstring(T t)
    {
        return std::to_string(t);
    }

#endif


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

    return std::string(pStr);
}

inline
std::wstring FromUtf8(const std::string &str)
{
    #if WCHAR_MAX <= 0xFFFFu /* wchar_t is 16 bit width, signed or unsigned */

        std::basic_string<utf32_char_t> stringU32 = utf32_from_utf8(str);
        std::basic_string<utf16_char_t> stringU16 = utf16_from_utf32(stringU32);
        return wstring_from_utf16(stringU16);

    #else

        return wstring32_from_utf8(str);

    #endif

}

inline
std::wstring FromUtf8(const char *pStr)
{
    if (!pStr)
    {
        return std::wstring();
    }

    return FromUtf8(std::string(pStr));
}

inline
std::wstring FromUtf8(const std::wstring &str)
{
    return str;
}

inline
std::wstring FromUtf8(const wchar_t *pStr)
{
    if (!pStr)
    {
        return std::wstring();
    }

    return std::wstring(pStr);
}


#ifdef SQUNICODE

    inline
    std::wstring ToSqString(const std::wstring &s)
    {
        return s;
    }

    inline
    std::wstring ToSqString(const wchar_t *s)
    {
        if (!s)
        {
            return std::wstring();
        }

        return ToSqString(std::wstring(s));
    }

    inline
    std::wstring ToSqString(const std::string &s)
    {
        return FromUtf8(s);
    }

    inline
    std::wstring ToSqString(const char *s)
    {
        if (!s)
        {
            return std::wstring();
        }

        return ToSqString(std::string(s));
    }

#else

    inline
    std::string ToSqString(const std::string &s)
    {
        return s;
    }

    inline
    std::string ToSqString(const char *s)
    {
        if (!s)
        {
            return std::string();
        }

        return std::string(s);
    }

    inline
    std::string ToSqString(const std::wstring &s)
    {
        return ToUtf8(s);
    }

    inline
    std::string ToSqString(const wchar_t *s)
    {
        if (!s)
        {
            return std::string();
        }

        return ToSqString(std::wstring(s));
    }


#endif


} // namespace ssq {


#endif /* SSQ_HELPERS_HEADER_H */