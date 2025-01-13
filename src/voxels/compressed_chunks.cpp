#include "compressed_chunks.hpp"

#include "coders/rle.hpp"
#include "coders/gzip.hpp"
#include "coders/byte_utils.hpp"
#include "voxels/Chunk.hpp"

inline constexpr int HAS_VOXELS = 0x1;
inline constexpr int HAS_METADATA = 0x2;

std::vector<ubyte> compressed_chunks::encode(const Chunk& chunk) {
    auto data = chunk.encode();

    /// world.get_chunk_data is only available in the main Lua state
    static util::Buffer<ubyte> rleBuffer;
    if (rleBuffer.size() < CHUNK_DATA_LEN * 2) {
        rleBuffer = util::Buffer<ubyte>(CHUNK_DATA_LEN * 2);
    }
    size_t rleCompressedSize =
        extrle::encode16(data.get(), CHUNK_DATA_LEN, rleBuffer.data());

    const auto gzipCompressedData = gzip::compress(
        rleBuffer.data(), rleCompressedSize
    );
    auto metadataBytes = chunk.blocksMetadata.serialize();

    ByteBuilder builder(2 + 8 + gzipCompressedData.size() + metadataBytes.size());
    builder.put(HAS_VOXELS | HAS_METADATA); // flags
    builder.put(0); // reserved
    builder.putInt32(gzipCompressedData.size());
    builder.put(gzipCompressedData.data(), gzipCompressedData.size());
    builder.putInt32(metadataBytes.size());
    builder.put(metadataBytes.data(), metadataBytes.size());
    return builder.build();
}

void compressed_chunks::decode(Chunk& chunk, const ubyte* src, size_t size) {
    ByteReader reader(src, size);

    ubyte flags = reader.get();
    reader.skip(1); // reserved byte

    if (flags & HAS_VOXELS) {
        size_t gzipCompressedSize = reader.getInt32();
        
        auto rleData = gzip::decompress(reader.pointer(), gzipCompressedSize);
        reader.skip(gzipCompressedSize);

        /// world.get_chunk_data is only available in the main Lua state
        static util::Buffer<ubyte> voxelData (CHUNK_DATA_LEN);
        extrle::decode16(rleData.data(), rleData.size(), voxelData.data());
        chunk.decode(voxelData.data());
        chunk.updateHeights();
    }
    if (flags & HAS_METADATA) {
        size_t metadataSize = reader.getInt32();
        chunk.blocksMetadata.deserialize(reader.pointer(), metadataSize);
        reader.skip(metadataSize);
    }
    chunk.setModifiedAndUnsaved();
}
