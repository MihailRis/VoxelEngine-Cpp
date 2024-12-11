#pragma once

#include <mutex>
#include <memory>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class Chunk;
class Level;

class ChunksStorage {
    Level* level;
    std::mutex mutex;
    std::unordered_map<glm::ivec2, std::weak_ptr<Chunk>> chunksMap;
public:
    ChunksStorage(Level* level);
    ~ChunksStorage() = default;

    std::shared_ptr<Chunk> fetch(int x, int z);
    std::shared_ptr<Chunk> create(int x, int z);
};
