#ifndef UTIL_STRINGUTIL_H_
#define UTIL_STRINGUTIL_H_

#include <string>
#include "../typedefs.h"

extern std::wstring lfill(std::wstring s, int length, wchar_t c);
extern std::wstring rfill(std::wstring s, int length, wchar_t c);

extern uint encode_utf8(uint c, ubyte* bytes);
extern std::string wstr2str_utf8(const std::wstring ws);

#endif // UTIL_STRINGUTIL_H_