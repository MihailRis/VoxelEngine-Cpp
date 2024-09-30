#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <limits>
#include <stdexcept>

#include "Buffer.hpp"
#include "data_io.hpp"

namespace util {    
    template<typename T>
    inline T read_int_le(const uint8_t* src, size_t offset=0) {
        return dataio::le2h(*(reinterpret_cast<const T*>(src) + offset));
    }

    // TODO: make it safer (minimize raw temporary pointers use)
    /// @brief Simple heap implementation for memory-optimal sparse array of 
    /// small different structures
    /// @note alignment is not impemented 
    /// (impractical in the context of scripting and memory consumption)
    /// @tparam Tindex entry index type
    /// @tparam Tsize entry size type
    template <typename Tindex, typename Tsize>
    class SmallHeap {
        std::vector<uint8_t> buffer;
        Tindex entriesCount;
    public:
        SmallHeap() : entriesCount(0) {}

        /// @brief Find current entry address by index
        /// @param index entry index
        /// @return temporary raw pointer or nullptr if entry does not exists
        /// @attention pointer becomes invalid after allocate(...) or free(...)
        uint8_t* find(Tindex index) {
            auto data = buffer.data();
            for (size_t i = 0; i < entriesCount; i++) {
                auto nextIndex = read_int_le<Tindex>(data);
                data += sizeof(Tindex);
                auto nextSize = read_int_le<Tsize>(data);
                data += sizeof(Tsize);
                if (nextIndex == index) {
                    return data;
                } else if (nextIndex > index) {
                    return nullptr;
                }
                data += nextSize;
            }
            return nullptr;
        }

        /// @brief Erase entry from the heap
        /// @param ptr valid entry pointer
        void free(uint8_t* ptr) {
            if (ptr == nullptr) {
                return;
            }
            auto entrySize = sizeOf(ptr);
            auto begin =
                buffer.begin() +
                ((ptr - sizeof(Tsize) - sizeof(Tindex)) - buffer.data());
            buffer.erase(
                begin, begin + entrySize + sizeof(Tsize) + sizeof(Tindex)
            );
            entriesCount--;
        }

        /// @brief Create or update entry (size)
        /// @param index entry index
        /// @param size entry size
        /// @return temporary entry pointer 
        /// @attention pointer becomes invalid after allocate(...) or free(...)
        uint8_t* allocate(Tindex index, size_t size) {
            const auto maxSize = std::numeric_limits<Tsize>::max();
            if (size > maxSize) {
                throw std::invalid_argument(
                    "requested "+std::to_string(size)+" bytes but limit is "+
                    std::to_string(maxSize));
            }
            if (size == 0) {
                throw std::invalid_argument("zero size");
            }
            ptrdiff_t offset = 0;
            if (auto found = find(index)) {
                auto entrySize = sizeOf(found);
                if (size == entrySize) {
                    std::memset(found, 0, entrySize);
                    return found;
                }
                this->free(found);
                return allocate(index, size);
            }
            for (size_t i = 0; i < entriesCount; i++) {
                auto data = buffer.data() + offset;
                auto nextIndex = read_int_le<Tindex>(data);
                data += sizeof(Tindex);
                auto nextSize = read_int_le<Tsize>(data);
                data += sizeof(Tsize);
                if (nextIndex > index) {
                    break;
                }
                data += nextSize;
                offset = data - buffer.data();
            }
            buffer.insert(
                buffer.begin() + offset,
                size + sizeof(Tindex) + sizeof(Tsize),
                0
            );
            entriesCount++;

            auto data = buffer.data() + offset;
            *reinterpret_cast<Tindex*>(data) = dataio::h2le(index);
            data += sizeof(Tindex);
            *reinterpret_cast<Tsize*>(data) = dataio::h2le(size);
            return data + sizeof(Tsize);
        }

        /// @param ptr valid entry pointer
        /// @return entry size
        Tsize sizeOf(uint8_t* ptr) const {
            if (ptr == nullptr) {
                return 0;
            }
            return read_int_le<Tsize>(ptr, -1);
        }

        /// @return number of entries
        Tindex count() const {
            return entriesCount;
        }

        /// @return total used bytes including entries metadata
        size_t size() const {
            return buffer.size();
        }

        inline bool operator==(const SmallHeap<Tindex, Tsize>& o) const {
            if (o.entriesCount != entriesCount) {
                return false;
            }
            return buffer == o.buffer;
        }

        util::Buffer<uint8_t> serialize() const {
            util::Buffer<uint8_t> out(sizeof(Tindex) + buffer.size());
            ubyte* dst = out.data();
            const ubyte* src = buffer.data();

            *reinterpret_cast<Tindex*>(dst) = dataio::h2le(entriesCount);
            dst += sizeof(Tindex);

            std::memcpy(dst, src, buffer.size());
            return out;
        }

        void deserialize(const ubyte* src, size_t size) {
            entriesCount = read_int_le<Tindex>(src);
            buffer.resize(size - sizeof(Tindex));
            std::memcpy(buffer.data(), src + sizeof(Tindex), buffer.size());
        }
    };
}
