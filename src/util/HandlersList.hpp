#pragma once

#include <mutex>
#include <iostream>

#include "typedefs.hpp"
#include "delegates.hpp"

namespace util {
    template<class...Types>
    class HandlersList {
        int nextid = 1;
        std::unordered_map<int, std::function<bool(Types...)>> handlers;
        std::vector<int> order;
        std::mutex mutex;
    public:
        HandlersList() = default;

        HandlersList(HandlersList&& o) {
            handlers = std::move(o.handlers);
            order = std::move(o.order);
            nextid = o.nextid;
        }

        void operator=(HandlersList&& o) {
            handlers = std::move(o.handlers);
            order = std::move(o.order);
            nextid = o.nextid;
        }

        observer_handler add(std::function<bool(Types...)> handler) {
            std::lock_guard lock(mutex);
            int id = nextid++;
            handlers[id] = std::move(handler);
            order.push_back(id);
            return observer_handler(new int(id), [this](int* id) { //-V508
                std::lock_guard lock(mutex);
                handlers.erase(*id);
                order.erase(
                    std::remove(order.begin(), order.end(), *id), order.end()
                );
                delete id;
            });
        }

        void notify(Types...args) {
            std::lock_guard lock(mutex);
            for (auto it = order.rbegin(); it != order.rend(); ++it) {
                if (handlers.at(*it)(args...)) {
                    break;
                }
            }
        }
    };
}
