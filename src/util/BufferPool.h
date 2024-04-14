#ifndef UTIL_BUFFER_POOL_H_
#define UTIL_BUFFER_POOL_H_

#include "../typedefs.h"

#include <queue>
#include <mutex>
#include <vector>
#include <memory>

namespace util {
    template<class T>
    class BufferPool {
        std::vector<std::unique_ptr<T[]>> buffers;
        std::queue<T*> freeBuffers;
        std::mutex mutex;
        size_t bufferSize;
    public:
        BufferPool(size_t bufferSize) : bufferSize(bufferSize) {
        }

        std::shared_ptr<T[]> get() {
            std::lock_guard lock(mutex);
            if (freeBuffers.empty()) {
                buffers.emplace_back(std::make_unique<T[]>(bufferSize));
                freeBuffers.push(buffers[buffers.size()-1].get());
            }
            auto* buffer = freeBuffers.front();
            freeBuffers.pop();
            return std::shared_ptr<T[]>(buffer, [this](T* ptr) {
                freeBuffers.push(ptr);
            });
        }
    };
}

#endif // UTIL_BUFFER_POOL_H_
