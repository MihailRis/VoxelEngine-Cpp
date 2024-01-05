#include "stringutil.h"

#include <vector>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <algorithm>

std::wstring util::lfill(std::wstring s, uint length, wchar_t c) {
    if (s.length() >= length) {
        return s;
    }
    std::wstringstream ss;
    for (uint i = 0; i < length-s.length(); i++) {
        ss << c;
    }
    ss << s;
    return ss.str();
}

std::wstring util::rfill(std::wstring s, uint length, wchar_t c) {
    if (s.length() >= length) {
        return s;
    }
    std::wstringstream ss;
    ss << s;
    for (uint i = 0; i < length-s.length(); i++) {
        ss << c;
    }
    return ss.str();
}

uint util::encode_utf8(uint32_t c, u_char8* bytes) {
    if (c < 0x80) {
        bytes[0] = ((c >> 0) & 0x7F) | 0x00;
        return 1;
    } else if (c < 0x0800) {
        bytes[0] = ((c >> 6) & 0x1F) | 0xC0;
        bytes[1] = ((c >> 0) & 0x3F) | 0x80;
        return 2;
    } else if (c < 0x010000) {
        bytes[0] = ((c >> 12) & 0x0F) | 0xE0;
        bytes[1] = ((c >> 6) & 0x3F) | 0x80;
        bytes[2] = ((c >> 0) & 0x3F) | 0x80;
        return 3;
    } else {
        bytes[0] = ((c >> 18) & 0x07) | 0xF0;
        bytes[1] = ((c >> 12) & 0x3F) | 0x80;
        bytes[2] = ((c >> 6) & 0x3F) | 0x80;
        bytes[3] = ((c >> 0) & 0x3F) | 0x80;
        return 4;
    }
}

struct utf_t {
	char mask;
	char lead;
	uint32_t beg;
	uint32_t end;
	int bits_stored;
};

const utf_t utf[] = {
	/*             mask        lead        beg      end       bits */
	{(char)0b00111111, (char)0b10000000, 0,       0,        6},
	{(char)0b01111111, (char)0b00000000, 0000,    0177,     7},
	{(char)0b00011111, (char)0b11000000, 0200,    03777,    5},
	{(char)0b00001111, (char)0b11100000, 04000,   0177777,  4},
	{(char)0b00000111, (char)0b11110000, 0200000, 04177777, 3},
	{0, 0, 0, 0, 0},
};


inline uint utf8_len(u_char8 cp) {
    uint len = 0;
	for (const utf_t* u = utf; u->mask; ++u) {
		if((cp >= u->beg) && (cp <= u->end)) {
			break;
		}
		++len;
	}
	if(len > 4) /* Out of bounds */
		throw std::runtime_error("utf-8 decode error");

	return len;
}

extern uint32_t util::decode_utf8(uint& size, const char* chr) {
	size = utf8_len(*chr);
	int shift = utf[0].bits_stored * (size - 1);
	uint32_t code = (*chr++ & utf[size].mask) << shift;

	for(uint i = 1; i < size; ++i, ++chr) {
		shift -= utf[0].bits_stored;
		code |= ((char)*chr & utf[0].mask) << shift;
	}
    return code;
}

std::string util::wstr2str_utf8(const std::wstring ws) {
    std::vector<char> chars;
    char buffer[4];
    for (wchar_t wc : ws) {
        uint size = encode_utf8((uint)wc, (u_char8*)buffer);
        for (uint i = 0; i < size; i++) {
            chars.push_back(buffer[i]);
        }
    }
    return std::string(chars.data(), chars.size());
}

std::wstring util::str2wstr_utf8(const std::string s) {
    std::vector<wchar_t> chars;
    size_t pos = 0;
    uint size = 0;
    while (pos < s.length()) {
        chars.push_back(decode_utf8(size, &s.at(pos)));
        pos += size;
    }
    return std::wstring(chars.data(), chars.size());
}

bool util::is_integer(std::string text) {
    for (char c : text) {
        if (c < '0' || c > '9')
            return false;
    }
    return true;
}

bool util::is_integer(std::wstring text) {
    for (wchar_t c : text) {
        if (c < L'0' || c > L'9')
            return false;
    }
    return true;
}

bool util::is_valid_filename(std::wstring name) {
    for (wchar_t c : name) {
        if (c < 31 || c == '/' || c == '\\' || c == '<' || c == '>' ||
            c == ':' || c == '"' || c == '|' || c == '?' || c == '*'){
            return false;
        }
    }
    return true;
}

void util::ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void util::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void util::trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}
