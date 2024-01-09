#ifndef FILES_RLE_H_
#define FILES_RLE_H_

#include "../typedefs.h"

namespace rle {
	
	template <class T>
	T decode(const T* src, T srclen, T* dst) {
		T offset = 0;
		for (T i = 0; i < srclen / sizeof(T);) {
			T len = src[i++];
			T c = src[i++];
			for (T j = 0; j <= len; j++) {
				dst[offset++] = c;
			}
		}
		return offset*sizeof(T);
	}

	template <class T>
	T encode(const T* src, T srclen, T* dst) {
		if (srclen == 0) {
			return 0;
		}
		T offset = 0;
		u_int counter = 0;
		T c = src[0];
		for (T i = 1; i < srclen / sizeof(T); i++) {
			T cnext = src[i];
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
		return offset*sizeof(T);
	}
}

#endif // FILES_RLE_H_
