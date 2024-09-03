#include "rle.hpp"

size_t rle::decode(const ubyte* src, size_t srclen, ubyte* dst) {
    size_t offset = 0;
    for (size_t i = 0; i < srclen;) {
        ubyte len = src[i++];
        ubyte c = src[i++];
        for (size_t j = 0; j <= len; j++) {
            dst[offset++] = c;
        }
    }
    return offset;
}

size_t rle::encode(const ubyte* src, size_t srclen, ubyte* dst) {
    if (srclen == 0) {
        return 0;
    }
    size_t offset = 0;
    ubyte counter = 0;
    ubyte c = src[0];
    for (size_t i = 1; i < srclen; i++) {
        ubyte cnext = src[i];
        if (cnext != c || counter == 255) {
            dst[offset++] = counter;
            dst[offset++] = c;
            c = cnext;
            counter = 0;
        } else {
            counter++;
        }
    }
    dst[offset++] = counter;
    dst[offset++] = c;
    return offset;
}

size_t rle::decode16(const ubyte* src, size_t srclen, ubyte* dst) {
    auto src16 = reinterpret_cast<const uint16_t*>(src);
    auto dst16 = reinterpret_cast<uint16_t*>(dst);
    size_t offset = 0;
    for (size_t i = 0; i < srclen / 2;) {
        uint16_t len = src16[i++];
        uint16_t c = src16[i++];
        for (size_t j = 0; j <= len; j++) {
            dst16[offset++] = c;
        }
    }
    return offset * 2;
}

size_t rle::encode16(const ubyte* src, size_t srclen, ubyte* dst) {
    if (srclen == 0) {
        return 0;
    }
    auto src16 = reinterpret_cast<const uint16_t*>(src);
    auto dst16 = reinterpret_cast<uint16_t*>(dst);
    size_t offset = 0;
    uint16_t counter = 0;
    uint16_t c = src16[0];
    for (size_t i = 1; i < srclen / 2; i++) {
        uint16_t cnext = src16[i];
        if (cnext != c || counter == 0xFFFF) {
            dst16[offset++] = counter;
            dst16[offset++] = c;
            c = cnext;
            counter = 0;
        } else {
            counter++;
        }
    }
    dst16[offset++] = counter;
    dst16[offset++] = c;
    return offset * 2;
}

size_t extrle::decode(const ubyte* src, size_t srclen, ubyte* dst) {
    size_t offset = 0;
    for (size_t i = 0; i < srclen;) {
        uint len = src[i++];
        if (len & 0x80) {
            len &= 0x7F;
            len |= ((uint)src[i++]) << 7;
        }
        ubyte c = src[i++];
        for (size_t j = 0; j <= len; j++) {
            dst[offset++] = c;
        }
    }
    return offset;
}

size_t extrle::encode(const ubyte* src, size_t srclen, ubyte* dst) {
    if (srclen == 0) {
        return 0;
    }
    size_t offset = 0;
    uint counter = 0;
    ubyte c = src[0];
    for (size_t i = 1; i < srclen; i++) {
        ubyte cnext = src[i];
        if (cnext != c || counter == max_sequence) {
            if (counter >= 0x80) {
                dst[offset++] = 0x80 | (counter & 0x7F);
                dst[offset++] = counter >> 7;
            } else {
                dst[offset++] = counter;
            }
            dst[offset++] = c;
            c = cnext;
            counter = 0;
        } else {
            counter++;
        }
    }
    if (counter >= 0x80) {
        dst[offset++] = 0x80 | (counter & 0x7F);
        dst[offset++] = counter >> 7;
    } else {
        dst[offset++] = counter;
    }
    dst[offset++] = c;
    return offset;
}
