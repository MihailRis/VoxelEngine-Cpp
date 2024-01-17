#include "gzip.h"

#define ZLIB_CONST
#include <zlib.h>
#include <math.h>
#include <memory>
#include "byte_utils.h"

std::vector<ubyte> gzip::compress(const ubyte* src, size_t size) {
    size_t buffer_size = 23+size*1.01;
    std::vector<ubyte> buffer;
    buffer.resize(buffer_size);

    // zlib struct
    z_stream defstream {};
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;
    defstream.avail_in = size;
    defstream.next_in = src;
    defstream.avail_out = buffer_size;
    defstream.next_out = buffer.data();

    // compression
    deflateInit2(&defstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 
                 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);

    size_t compressed_size = defstream.next_out - buffer.data();
    buffer.resize(compressed_size);
    return buffer;
}

std::vector<ubyte> gzip::decompress(const ubyte* src, size_t size) {
    // getting uncompressed data length from gzip footer
    size_t decompressed_size = *(uint32_t*)(src+size-4);
    std::vector<ubyte> buffer;
    buffer.resize(decompressed_size);

    // zlib struct
    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
    infstream.avail_in = size;
    infstream.next_in = src;
    infstream.avail_out = decompressed_size;
    infstream.next_out = buffer.data();

    inflateInit2(&infstream, 16+MAX_WBITS);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);

    return buffer;
}
