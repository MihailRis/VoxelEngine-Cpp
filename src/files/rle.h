#ifndef FILES_RLE_H_
#define FILES_RLE_H_

#include "../typedefs.h"

namespace rle {
	size_t encode(const u_char8* src, size_t length, u_char8* dst);
	size_t decode(const u_char8* src, size_t length, u_char8* dst);
}

namespace extrle {
	constexpr uint max_sequence = 0x7FFF;
	size_t encode(const u_char8* src, size_t length, u_char8* dst);
	size_t decode(const u_char8* src, size_t length, u_char8* dst);
}

#endif // FILES_RLE_H_
