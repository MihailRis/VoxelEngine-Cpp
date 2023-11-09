#include "stringutil.h"

#include <vector>
#include <locale>
#include <sstream>

using std::vector;
using std::string;
using std::stringstream;
using std::wstring;
using std::wstringstream;

wstring lfill(wstring s, int length, wchar_t c) {
    if (s.length() >= length) {
        return s;
    }
    wstringstream ss;
    for (int i = 0; i < length-s.length(); i++) {
        ss << c;
    }
    ss << s;
    return ss.str();
}

wstring rfill(wstring s, int length, wchar_t c) {
    if (s.length() >= length) {
        return s;
    }
    wstringstream ss;
    ss << s;
    for (int i = 0; i < length-s.length(); i++) {
        ss << c;
    }
    return ss.str();
}

uint encode_utf8(uint c, ubyte* bytes) {
    if (c < 0x80) {
        bytes[0] = c >> 0 & 0x7F | 0x00;
        return 1;
    } else if (c < 0x0800) {
        bytes[0] = c >> 6 & 0x1F | 0xC0;
        bytes[1] = c >> 0 & 0x3F | 0x80;
        return 2;
    } else if (c < 0x010000) {
        bytes[0] = c >> 12 & 0x0F | 0xE0;
        bytes[1] = c >> 6 & 0x3F | 0x80;
        bytes[2] = c >> 0 & 0x3F | 0x80;
        return 3;
    } else {
        bytes[0] = c >> 18 & 0x07 | 0xF0;
        bytes[1] = c >> 12 & 0x3F | 0x80;
        bytes[2] = c >> 6 & 0x3F | 0x80;
        bytes[3] = c >> 0 & 0x3F | 0x80;
        return 4;
    }
}

string wstr2str_utf8(const wstring ws) {
    vector<char> chars;
    char buffer[4];
    for (wchar_t wc : ws) {
        uint size = encode_utf8((uint)wc, (ubyte*)buffer);
        for (uint i = 0; i < size; i++) {
            chars.push_back(buffer[i]);
        }
    }
    return string(chars.data(), chars.size());
}