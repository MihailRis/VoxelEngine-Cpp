#include "LevelEvents.hpp"

using std::vector;

void LevelEvents::listen(lvl_event_type type, const event_func& func) {
    auto& callbacks = this->callbacks[type];
    callbacks.push_back(func);
}

void LevelEvents::trigger(lvl_event_type type, void* data) {
    const auto& callbacks = this->callbacks[type];
    for (const event_func& func : callbacks) {
        func(type, data);
    }
}
