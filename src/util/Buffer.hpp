#pragma once

#include <memory>
#include <cstring>
#include <initializer_list>

namespace util {
    /// @brief Template similar to std::unique_ptr stores a buffer with its size
    /// @tparam T buffer elements type
    template<typename T>
    class Buffer {
        std::unique_ptr<T[]> ptr;
        size_t length;
    public:
        Buffer() = default;

        Buffer(size_t length)
         : ptr(std::make_unique<T[]>(length)), length(length) {
        }
        explicit Buffer(const Buffer<T>& o) : Buffer(o.data(), o.size()) {}

        Buffer(Buffer<T>&& o) : ptr(std::move(o.ptr)), length(o.length) {}

        Buffer(std::unique_ptr<T[]> ptr, size_t length)
         : ptr(std::move(ptr)), length(length) {}

        Buffer(const T* src, size_t length)
         : ptr(std::make_unique<T[]>(length)), length(length) {
            std::memcpy(ptr.get(), src, length * sizeof(T));
        }

        Buffer(std::initializer_list<T> values)
         : ptr(std::make_unique<T[]>(values.size())), 
           length(values.size()) {
            std::copy(values.begin(), values.end(), ptr.get());
        }

        Buffer(std::nullptr_t) noexcept : ptr(nullptr), length(0) {}

        Buffer& operator=(Buffer&&) = default;

        inline bool operator==(std::nullptr_t) const noexcept {
            return ptr == nullptr;
        }

        inline bool operator!=(std::nullptr_t) const noexcept {
            return ptr != nullptr;
        }

        T& operator[](long long index) {
            return ptr[index];
        }

        const T& operator[](long long index) const {
            return ptr[index];
        }

        T* data() {
            return ptr.get();
        }

        const T* data() const {
            return ptr.get();
        }

        size_t size() const {
            return length;
        }

        /// @brief Take ownership over the buffer unique_ptr
        std::unique_ptr<T[]> release() {
            return std::move(ptr);
        }

        /// @brief Create a buffer copy 
        Buffer clone() const {
            return Buffer(ptr.get(), length);
        }

        /// @brief Update buffer size without releasing used memory
        /// @param size new size (must be less or equal to current)
        void resizeFast(size_t size) {
            length = size;
        }

        const T* begin() const {
            return ptr.get();
        }

        const T* end() const {
            return ptr.get() + length;
        }
    };
}
