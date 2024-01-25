#ifndef CODERS_GZIP_H_
#define CODERS_GZIP_H_

#include <vector>
#include "../typedefs.h"

namespace gzip {
    const unsigned char MAGIC[] = "\x1F\x8B";

    /* Compress bytes array to GZIP format
     @param src source bytes array
     @param size length of source bytes array */
    std::vector<ubyte> compress(const ubyte* src, size_t size);
    
    /* Decompress bytes array from GZIP 
     @param src GZIP data
     @param size length of GZIP data */
    std::vector<ubyte> decompress(const ubyte* src, size_t size);
}

#endif // CODERS_GZIP_H_
