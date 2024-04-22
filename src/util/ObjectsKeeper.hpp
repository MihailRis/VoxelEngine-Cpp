#ifndef UTIL_OBJECTS_KEEPER_HPP_
#define UTIL_OBJECTS_KEEPER_HPP_

#include <vector>
#include <memory>

namespace util {
    /// @brief Keeps shared pointers alive until destruction
    class ObjectsKeeper {
        std::vector<std::shared_ptr<void>> ptrs;
    public:
        virtual ~ObjectsKeeper() {}

        virtual void keepAlive(std::shared_ptr<void> ptr) {
            ptrs.push_back(ptr);
        }
    };
}

#endif // UTIL_OBJECTS_KEEPER_HPP_
