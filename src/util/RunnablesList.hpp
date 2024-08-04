#ifndef UTIL_RUNNABLES_LIST_HPP_
#define UTIL_RUNNABLES_LIST_HPP_

#include <memory>
#include <unordered_map>
#include <utility>

#include <delegates.hpp>
#include <typedefs.hpp>

namespace util {
    class RunnablesList {
        int nextid = 1;
        std::unordered_map<int, runnable> runnables;
    public:
        observer_handler add(runnable callback) {
            int id = nextid++;
            runnables[id] = std::move(callback);
            return observer_handler(new int(id), [this](int* id) { //-V508
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

#endif  // UTIL_RUNNABLES_LIST_HPP_
