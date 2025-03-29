#pragma once

#include "typedefs.hpp"
#include "Chunk.hpp"
#include "coders/byte_utils.hpp"

#include <vector>

class ContentIndices;
class WorldRegions;

namespace compressed_chunks {
    std::vector<ubyte> encode(
        const ubyte* voxelData,
        const BlocksMetadata& metadata,
        util::Buffer<ubyte>& rleBuffer
    );
    std::vector<ubyte> encode(const Chunk& chunk);
    void decode(
        Chunk& chunk,
        const ubyte* src,
        size_t size,
        const ContentIndices& indices
    );
    void save(int x, int z, std::vector<ubyte> bytes, WorldRegions& regions);
}
