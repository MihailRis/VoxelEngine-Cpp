#ifndef UTIL_STRINGUTIL_H_
#define UTIL_STRINGUTIL_H_

#include <string>
#include <vector>
#include "../typedefs.h"

namespace util {
    /// @brief Function used for string serialization in text formats 
    extern std::string escape(const std::string& s);

    /// @brief Function used for error messages
    extern std::string quote(const std::string& s);

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

    extern std::string to_string(double x);
    extern std::wstring to_wstring(double x, int precision);

    extern std::string base64_encode(const ubyte* data, size_t size);
    extern std::vector<ubyte> base64_decode(const char* str, size_t size);
    extern std::vector<ubyte> base64_decode(const std::string& str);

    extern std::string mangleid(uint64_t value);

    extern int replaceAll(std::string& str, const std::string& from, const std::string& to);

    extern double parse_double(const std::string& str);
    extern double parse_double(const std::string& str, size_t offset, size_t len);

    extern std::wstring lower_case(const std::wstring& str);
    extern std::wstring upper_case(const std::wstring& str);
    extern std::wstring capitalized(const std::wstring& str);
    extern std::wstring pascal_case(const std::wstring& str);

    /// @brief Convert `any_prefix:some_data_id` to `some data id`. Leaves
    /// '_' characters at end of the id.
    /// @param id source id
    /// @return resulting caption or empty string if there's nothing but prefix
    extern std::string id_to_caption(const std::string& id);

    extern std::vector<std::string> split(const std::string& str, char delimiter);
    extern std::vector<std::wstring> split(const std::wstring& str, char delimiter);
}

#endif // UTIL_STRINGUTIL_H_
