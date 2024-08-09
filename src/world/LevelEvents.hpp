#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

class Chunk;

enum lvl_event_type {
    EVT_CHUNK_HIDDEN,
};

using chunk_event_func = std::function<void(lvl_event_type, Chunk*)>;

class LevelEvents {
    std::unordered_map<lvl_event_type, std::vector<chunk_event_func>>
        chunk_callbacks;
public:
    void listen(lvl_event_type type, const chunk_event_func& func);
    void trigger(lvl_event_type type, Chunk* chunk);
};
