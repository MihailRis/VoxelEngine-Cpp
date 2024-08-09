#pragma once

#include <memory>
#include <vector>

namespace util {
    /// @brief Keeps shared pointers alive until destruction
    class ObjectsKeeper {
        std::vector<std::shared_ptr<void>> ptrs;
    public:
        virtual ~ObjectsKeeper() {
        }

        virtual void keepAlive(std::shared_ptr<void> ptr) {
            ptrs.push_back(ptr);
        }
    };
}
