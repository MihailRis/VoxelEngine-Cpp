#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "util/WeakPtrsMap.hpp"

class Chunk;
class Level;

class ChunksStorage {
    Level* level;
    std::shared_ptr<util::WeakPtrsMap<glm::ivec2, Chunk>> chunksMap;
public:
    ChunksStorage(Level* level);
    ~ChunksStorage() = default;

    std::shared_ptr<Chunk> fetch(int x, int z);
    std::shared_ptr<Chunk> create(int x, int z);
};
