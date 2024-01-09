#ifndef UTIL_STRINGUTIL_H_
#define UTIL_STRINGUTIL_H_

#include <string>
#include "../typedefs.h"

namespace util {
    extern std::wstring lfill(std::wstring s, u_int length, wchar_t c);
    extern std::wstring rfill(std::wstring s, u_int length, wchar_t c);

    extern u_int encode_utf8(u_int c, u_char* bytes);
    extern u_int decode_utf8(u_int& size, const char* bytes);
    extern std::string wstr2str_utf8(const std::wstring ws);
    extern std::wstring str2wstr_utf8(const std::string s);
    extern bool is_integer(std::string text);
    extern bool is_integer(std::wstring text);
    extern bool is_valid_filename(std::wstring name);

    extern void ltrim(std::string &s);
    extern void rtrim(std::string &s);
    extern void trim(std::string &s);
}

#endif // UTIL_STRINGUTIL_H_