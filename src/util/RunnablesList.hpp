#ifndef UTIL_RUNNABLES_LIST_HPP_
#define UTIL_RUNNABLES_LIST_HPP_

#include "../typedefs.hpp"
#include "../delegates.hpp"

#include <memory>
#include <unordered_map>
#include <utility>

namespace util {
    class RunnablesList {
        int nextid = 1;
        std::unordered_map<int, runnable> runnables;
    public:
        observer_handler add(runnable callback) {
            int id = nextid++;
            runnables[id] = std::move(callback);
            return observer_handler(new int(id), [this](int* id) {
                runnables.erase(*id);
                delete id;
            });
        }

        void notify() {
            for (auto& entry : runnables) {
                entry.second();
            }
        }
    };
}

#endif // UTIL_RUNNABLES_LIST_HPP_
