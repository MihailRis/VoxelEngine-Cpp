#ifndef CODERS_GZIP_H_
#define CODERS_GZIP_H_

#include <vector>
#include "../typedefs.h"

namespace gzip {
    const unsigned char MAGIC[] = "\x1F\x8B";
    std::vector<ubyte> compress(const ubyte* src, size_t size);
    std::vector<ubyte> decompress(const ubyte* src, size_t size);
}

#endif // CODERS_GZIP_H_
