#include "rle.h"
// template <class T>
// size_t rle::decode(const T* src, size_t srclen, T* dst) {
// 	size_t offset = 0;
// 	for (size_t i = 0; i < srclen;) {
// 		T len = src[i++];
// 		T c = src[i++];
// 		for (size_t j = 0; j <= len; j++) {
// 			dst[offset++] = c;
// 		}
// 	}
// 	return offset;
// }

// template <class T>
// size_t rle::encode(const T* src, size_t srclen, T* dst) {
// 	if (srclen == 0) {
// 		return 0;
// 	}
// 	size_t offset = 0;
// 	u_int counter = 0;
// 	T c = src[0];
// 	for (size_t i = 1; i < srclen; i++) {
// 		T cnext = src[i];
// 		if (cnext != c || counter == 255) {
// 			dst[offset++] = counter;
// 			dst[offset++] = c;
// 			c = cnext;
// 			counter = 0;
// 		}
// 		else {
// 			counter++;
// 		}
// 	}
// 	dst[offset++] = counter;
// 	dst[offset++] = c;
// 	return offset;
// }


// template <class T>
// size_t extrle::decode(const T* src, size_t srclen, T* dst) {
// 	size_t offset = 0;
// 	for (size_t i = 0; i < srclen;) {
// 		T len = src[i++];
// 		if (len & 0x80) {
// 			len &= 0x7F;
// 			len |= ((T)src[i++]) << 7;
// 		}
// 		T c = src[i++];
// 		for (size_t j = 0; j <= len; j++) {
// 			dst[offset++] = c;
// 		}
// 	}
// 	return offset;
// }

// template <class T>
// size_t extrle::encode(const T* src, size_t srclen, T* dst) {
// 	if (srclen == 0) {
// 		return 0;
// 	}
// 	size_t offset = 0;
// 	u_int counter = 0;
// 	T c = src[0];
// 	for (size_t i = 1; i < srclen; i++) {
// 		T cnext = src[i];
// 		if (cnext != c || counter == max_sequence) {
// 			if (counter >= 0x80) {
// 				dst[offset++] = 0x80 | (counter & 0x7F);
// 				dst[offset++] = counter >> 7;
// 			}
// 			else {
// 				dst[offset++] = counter;
// 			}
// 			dst[offset++] = c;
// 			c = cnext;
// 			counter = 0;
// 		}
// 		else {
// 			counter++;
// 		}
// 	}
// 	if (counter >= 0x80) {
// 		dst[offset++] = 0x80 | (counter & 0x7F);
// 		dst[offset++] = counter >> 7;
// 	}
// 	else {
// 		dst[offset++] = counter;
// 	}
// 	dst[offset++] = c;
// 	return offset;
// }
