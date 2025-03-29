#include "compatibility.hpp"

#include <stdexcept>

#include "constants.hpp"
#include "voxels/voxel.hpp"
#include "coders/compression.hpp"
#include "coders/byte_utils.hpp"
#include "lighting/Lightmap.hpp"
#include "util/data_io.hpp"

static inline size_t VOXELS_DATA_SIZE_V1 = CHUNK_VOL * 4;
static inline size_t VOXELS_DATA_SIZE_V2 = CHUNK_VOL * 4;

static util::Buffer<ubyte> convert_voxels_1to2(const ubyte* buffer, uint32_t size) {
    auto data = compression::decompress(
        buffer, size, VOXELS_DATA_SIZE_V1, compression::Method::EXTRLE8);

    util::Buffer<ubyte> dstBuffer(VOXELS_DATA_SIZE_V2);
    auto dst = reinterpret_cast<uint16_t*>(dstBuffer.data());

    for (size_t i = 0; i < CHUNK_VOL; i++) {
        ubyte bid1 = data[i];
        ubyte bid2 = data[CHUNK_VOL + i];

        ubyte bst1 = data[CHUNK_VOL * 2 + i];
        ubyte bst2 = data[CHUNK_VOL * 3 + i];

        dst[i] =
            (static_cast<blockid_t>(bid1) << 8) | static_cast<blockid_t>(bid2);
        dst[CHUNK_VOL + i] = (
            (static_cast<blockstate_t>(bst1) << 8) |
             static_cast<blockstate_t>(bst2)
        );
    }
    size_t outLen;
    auto compressed = compression::compress(
        dstBuffer.data(), VOXELS_DATA_SIZE_V2, outLen, compression::Method::EXTRLE16);
    return util::Buffer<ubyte>(std::move(compressed), outLen);
}

util::Buffer<ubyte> compatibility::convert_region_2to3(
    const util::Buffer<ubyte>& src, RegionLayerIndex layer
) {
    const size_t REGION_CHUNKS = 1024;
    // const size_t HEADER_SIZE = 10;
    const size_t OFFSET_TABLE_SIZE = REGION_CHUNKS * sizeof(uint32_t);
    const ubyte COMPRESS_NONE = 0;
    const ubyte COMPRESS_EXTRLE8 = 1;
    const ubyte COMPRESS_EXTRLE16 = 2;

    const ubyte* const ptr = src.data();

    ByteBuilder builder;
    builder.putCStr(".VOXREG");
    builder.put(3);
    switch (layer) {
        case REGION_LAYER_VOXELS: builder.put(COMPRESS_EXTRLE16); break;
        case REGION_LAYER_LIGHTS: builder.put(COMPRESS_EXTRLE8); break;
        default: builder.put(COMPRESS_NONE); break;
    }

    uint32_t offsets[REGION_CHUNKS] {};

    auto tablePtr = reinterpret_cast<const uint32_t*>(
        ptr + src.size() - OFFSET_TABLE_SIZE
    );

    for (size_t i = 0; i < REGION_CHUNKS; i++) {
        uint32_t srcOffset = dataio::be2h(tablePtr[i]);
        if (srcOffset == 0) {
            continue;
        }
        uint32_t size = *reinterpret_cast<const uint32_t*>(ptr + srcOffset);
        size = dataio::be2h(size);

        const ubyte* data = ptr + srcOffset + sizeof(uint32_t);
        offsets[i] = builder.size();

        switch (layer) {
            case REGION_LAYER_VOXELS: {
                auto dstdata = convert_voxels_1to2(data, size);
                builder.putInt32(dstdata.size());
                builder.putInt32(VOXELS_DATA_SIZE_V2);
                builder.put(dstdata.data(), dstdata.size());
                break;
            }
            case REGION_LAYER_LIGHTS:
                builder.putInt32(size);
                builder.putInt32(LIGHTMAP_DATA_LEN);
                builder.put(data, size);
                break;
            case REGION_LAYER_ENTITIES:
            case REGION_LAYER_INVENTORIES:
            case REGION_LAYER_BLOCKS_DATA: {
                builder.putInt32(size);
                builder.putInt32(size);
                builder.put(data, size);
                break;
            case REGION_LAYERS_COUNT: 
                throw std::invalid_argument("invalid enum");
            }
        }
    }
    for (size_t i = 0; i < REGION_CHUNKS; i++) {
        builder.putInt32(offsets[i]);
    }
    return util::Buffer<ubyte>(builder.build().data(), builder.size());
}
