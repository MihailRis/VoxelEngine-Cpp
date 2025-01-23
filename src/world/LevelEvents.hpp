#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

class Chunk;

enum class LevelEventType {
    CHUNK_SHOWN,
    CHUNK_HIDDEN,
    CHUNK_PRESENT,
    CHUNK_UNLOAD,
};

using ChunkEventFunc = std::function<void(LevelEventType, Chunk*)>;

class LevelEvents {
    std::unordered_map<LevelEventType, std::vector<ChunkEventFunc>>
        chunk_callbacks;
public:
    void listen(LevelEventType type, const ChunkEventFunc& func);
    void trigger(LevelEventType type, Chunk* chunk);
};
