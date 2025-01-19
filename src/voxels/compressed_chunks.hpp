#pragma once

#include "typedefs.hpp"

#include <vector>

class Chunk;
class WorldRegions;

namespace compressed_chunks {
    std::vector<ubyte> encode(const Chunk& chunk);
    void decode(Chunk& chunk, const ubyte* src, size_t size);
    void save(int x, int z, std::vector<ubyte> bytes, WorldRegions& regions);
}
