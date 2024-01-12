#ifndef UTIL_STRINGUTIL_H_
#define UTIL_STRINGUTIL_H_

#include <string>
#include "../typedefs.h"

namespace util {
    extern std::wstring lfill(std::wstring s, uint length, wchar_t c);
    extern std::wstring rfill(std::wstring s, uint length, wchar_t c);

    extern uint encode_utf8(uint32_t c, ubyte* bytes);
    extern uint32_t decode_utf8(uint& size, const char* bytes);
    extern std::string wstr2str_utf8(const std::wstring ws);
    extern std::wstring str2wstr_utf8(const std::string s);
    extern bool is_integer(std::string text);
    extern bool is_integer(std::wstring text);
    extern bool is_valid_filename(std::wstring name);

    extern void ltrim(std::string &s);
    extern void rtrim(std::string &s);
    extern void trim(std::string &s);

    extern std::wstring to_wstring(double x, int precision);
}

#endif // UTIL_STRINGUTIL_H_