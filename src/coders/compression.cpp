#include "compression.hpp"

#include <cstring>
#include <stdexcept>

#include "rle.hpp"
#include "gzip.hpp"
#include "util/BufferPool.hpp"

using namespace compression;

static util::BufferPool<ubyte> buffer_pools[] {
    {255},
    {UINT16_MAX},
    {UINT16_MAX * 8},
};

static std::shared_ptr<ubyte[]> get_buffer(size_t minSize) {
    for (auto& pool : buffer_pools) {
        if (minSize <= pool.getBufferSize()) {
            return pool.get();
        }
    }
    return nullptr;
}

std::unique_ptr<ubyte[]> compression::compress(
    const ubyte* src, size_t srclen, size_t& len, Method method
) {
    switch (method) {
        case Method::NONE:
            throw std::invalid_argument("compression method is NONE");
        case Method::EXTRLE8: {
            // max extrle out size is srcLen * 2
            auto buffer = get_buffer(srclen * 2);
            auto bytes = buffer.get();
            std::unique_ptr<ubyte[]> uptr;
            if (bytes == nullptr) {
                uptr = std::make_unique<ubyte[]>(srclen * 2);
                bytes = uptr.get();
            }
            len = extrle::encode(src, srclen, bytes);
            if (uptr) {
                return uptr;
            }
            auto data = std::make_unique<ubyte[]>(len);
            std::memcpy(data.get(), bytes, len);
            return data;
        }
        case Method::GZIP: {
            auto buffer = gzip::compress(src, srclen);
            auto data = std::make_unique<ubyte[]>(buffer.size());
            std::memcpy(data.get(), buffer.data(), buffer.size());
            len = buffer.size();
            return data;
        }
        default:
            throw std::runtime_error("not implemented");
    }
}

std::unique_ptr<ubyte[]> compression::decompress(
    const ubyte* src, size_t srclen, size_t dstlen, Method method
) {
    switch (method) {
        case Method::NONE:
            throw std::invalid_argument("compression method is NONE");
        case Method::EXTRLE8: {
            auto decompressed = std::make_unique<ubyte[]>(dstlen);
            extrle::decode(src, srclen, decompressed.get());
            return decompressed;
        }
        case Method::GZIP: {
            auto buffer = gzip::decompress(src, srclen);
            if (buffer.size() != dstlen) {
                throw std::runtime_error(
                    "expected decompressed size " + std::to_string(dstlen) +
                    " got " + std::to_string(buffer.size()));
            }
            auto decompressed = std::make_unique<ubyte[]>(buffer.size());
            std::memcpy(decompressed.get(), buffer.data(), buffer.size());
            return decompressed;
        }
        default:
            throw std::runtime_error("not implemented");
    }
}
