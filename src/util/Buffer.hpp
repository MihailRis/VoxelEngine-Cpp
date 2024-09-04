#pragma once

#include <memory>
#include <cstring>

namespace util {
    template<typename T>
    class Buffer {
        std::unique_ptr<T[]> ptr;
        size_t length;
    public:
        Buffer(size_t length)
         : ptr(std::make_unique<T[]>(length)), length(length) {
        }

        Buffer(std::unique_ptr<T[]> ptr, size_t length)
         : ptr(std::move(ptr)), length(length) {}

        Buffer(const T* src, size_t length)
         : ptr(std::make_unique<T[]>(length)), length(length) {
            std::memcpy(ptr.get(), src, length);
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

        std::unique_ptr<T[]> release() {
            return std::move(ptr);
        }

        Buffer clone() const {
            return Buffer(ptr.get(), length);
        }
    };
}
