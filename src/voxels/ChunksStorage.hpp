#pragma once

#include <memory>
#include <unordered_map>

#include "typedefs.hpp"
#include "voxel.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class Chunk;
class Level;

class ChunksStorage {
    Level* level;
    std::unordered_map<glm::ivec2, std::shared_ptr<Chunk>> chunksMap;
public:
    ChunksStorage(Level* level);
    ~ChunksStorage() = default;

    std::shared_ptr<Chunk> get(int x, int z) const;
    void store(const std::shared_ptr<Chunk>& chunk);
    void remove(int x, int y);
    std::shared_ptr<Chunk> create(int x, int z);
};
