#pragma once

#include "typedefs.hpp"

#include <vector>

class Chunk;

namespace compressed_chunks {
    std::vector<ubyte> encode(const Chunk& chunk);
    void decode(Chunk& chunk, const ubyte* src, size_t size);
}
