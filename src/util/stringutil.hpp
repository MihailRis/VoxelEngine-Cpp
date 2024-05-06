#ifndef UTIL_STRINGUTIL_HPP_
#define UTIL_STRINGUTIL_HPP_

#include "../typedefs.h"

#include <string>
#include <vector>

namespace util {
    /// @brief Function used for string serialization in text formats 
    std::string escape(const std::string& s);

    /// @brief Function used for error messages
    std::string quote(const std::string& s);

    std::wstring lfill(std::wstring s, uint length, wchar_t c);
    std::wstring rfill(std::wstring s, uint length, wchar_t c);

    uint encode_utf8(uint32_t c, ubyte* bytes);
    uint32_t decode_utf8(uint& size, const char* bytes);
    std::string wstr2str_utf8(const std::wstring ws);
    std::wstring str2wstr_utf8(const std::string s);
    bool is_integer(std::string text);
    bool is_integer(std::wstring text);
    bool is_valid_filename(std::wstring name);

    void ltrim(std::string &s);
    void rtrim(std::string &s);
    void trim(std::string &s);

    std::string to_string(double x);
    std::wstring to_wstring(double x, int precision);

    std::string base64_encode(const ubyte* data, size_t size);
    std::vector<ubyte> base64_decode(const char* str, size_t size);
    std::vector<ubyte> base64_decode(const std::string& str);

    std::string mangleid(uint64_t value);

    int replaceAll(std::string& str, const std::string& from, const std::string& to);

    double parse_double(const std::string& str);
    double parse_double(const std::string& str, size_t offset, size_t len);

    std::wstring lower_case(const std::wstring& str);
    std::wstring upper_case(const std::wstring& str);
    std::wstring capitalized(const std::wstring& str);
    std::wstring pascal_case(const std::wstring& str);

    /// @brief Convert `any_prefix:some_data_id` to `some data id`. Leaves
    /// '_' characters at end of the id.
    /// @param id source id
    /// @return resulting caption or empty string if there's nothing but prefix
    std::string id_to_caption(const std::string& id);

    std::vector<std::string> split(const std::string& str, char delimiter);
    std::vector<std::wstring> split(const std::wstring& str, char delimiter);

    std::string format_data_size(size_t size);
}

#endif // UTIL_STRINGUTIL_HPP_
