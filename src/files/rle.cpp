#include "rle.h"

size_t rle::decode(const u_char8* src, size_t srclen, u_char8* dst) {
	size_t offset = 0;
	for (size_t i = 0; i < srclen;) {
		u_char8 len = src[i++];
		u_char8 c = src[i++];
		for (size_t j = 0; j <= len; j++) {
			dst[offset++] = c;
		}
	}
	return offset;
}

size_t rle::encode(const u_char8* src, size_t srclen, u_char8* dst) {
	if (srclen == 0) {
		return 0;
	}
	size_t offset = 0;
	u_char8 counter = 0;
	u_char8 c = src[0];
	for (size_t i = 1; i < srclen; i++) {
		u_char8 cnext = src[i];
		if (cnext != c || counter == 255) {
			dst[offset++] = counter;
			dst[offset++] = c;
			c = cnext;
			counter = 0;
		}
		else {
			counter++;
		}
	}
	dst[offset++] = counter;
	dst[offset++] = c;
	return offset;
}


size_t extrle::decode(const u_char8* src, size_t srclen, u_char8* dst) {
	size_t offset = 0;
	for (size_t i = 0; i < srclen;) {
		uint len = src[i++];
		if (len & 0x80) {
			len &= 0x7F;
			len |= ((uint)src[i++]) << 7;
		}
		u_char8 c = src[i++];
		for (size_t j = 0; j <= len; j++) {
			dst[offset++] = c;
		}
	}
	return offset;
}

size_t extrle::encode(const u_char8* src, size_t srclen, u_char8* dst) {
	if (srclen == 0) {
		return 0;
	}
	size_t offset = 0;
	uint counter = 0;
	u_char8 c = src[0];
	for (size_t i = 1; i < srclen; i++) {
		u_char8 cnext = src[i];
		if (cnext != c || counter == max_sequence) {
			if (counter >= 0x80) {
				dst[offset++] = 0x80 | (counter & 0x7F);
				dst[offset++] = counter >> 7;
			}
			else {
				dst[offset++] = counter;
			}
			dst[offset++] = c;
			c = cnext;
			counter = 0;
		}
		else {
			counter++;
		}
	}
	if (counter >= 0x80) {
		dst[offset++] = 0x80 | (counter & 0x7F);
		dst[offset++] = counter >> 7;
	}
	else {
		dst[offset++] = counter;
	}
	dst[offset++] = c;
	return offset;
}
