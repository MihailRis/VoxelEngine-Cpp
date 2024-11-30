#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

enum lvl_event_type {
    EVT_CHUNK_HIDDEN,
    EVT_BLOCK_CHANGED,
};

class LevelEvents {
    using event_func = std::function<void(lvl_event_type, void*)>;
    std::unordered_map<lvl_event_type, std::vector<event_func>> callbacks;
public:
    void listen(lvl_event_type type, const event_func& func);
    void trigger(lvl_event_type type, void* data);
};
