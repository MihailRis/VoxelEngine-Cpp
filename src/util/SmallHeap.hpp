#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <stdexcept>

namespace util {
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
        /// @return nullptr if entry does not exists
        uint8_t* find(Tindex index) {
            auto data = buffer.data();
            for (size_t i = 0; i < entriesCount; i++) {
                auto nextIndex = *reinterpret_cast<Tindex*>(data);
                data += sizeof(Tindex);
                auto nextSize = *reinterpret_cast<Tsize*>(data);
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
        /// (invalid after next allocate(...) or free(...))
        uint8_t* allocate(Tindex index, Tsize size) {
            if (size == 0) {
                throw std::runtime_error("size is 0");
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
                auto nextIndex = *reinterpret_cast<Tindex*>(data);
                data += sizeof(Tindex);
                auto nextSize = *reinterpret_cast<Tsize*>(data);
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
            *reinterpret_cast<Tindex*>(data) = index;
            data += sizeof(Tindex);
            *reinterpret_cast<Tsize*>(data) = size;
            return data + sizeof(Tsize);
        }

        /// @param ptr valid entry pointer
        /// @return entry size
        Tsize sizeOf(uint8_t* ptr) const {
            if (ptr == nullptr) {
                return 0;
            }
            return *(reinterpret_cast<Tsize*>(ptr)-1);
        }

        /// @return number of entries
        Tindex count() const {
            return entriesCount;
        }

        /// @return total used bytes including entries metadata
        size_t size() const {
            return buffer.size();
        }
    };
}
