#pragma once

#include <memory>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "voxel.hpp"

class Chunk;
class Level;

class ChunksStorage {
    Level* level;
    std::unordered_map<long long, std::shared_ptr<Chunk>> chunksMap;
    std::unordered_map<glm::ivec2, std::shared_ptr<Chunk>> pinnedChunks;
    std::unordered_map<ptrdiff_t, int> refCounters;
public:
    ChunksStorage(Level* level);
    ~ChunksStorage() = default;

    std::shared_ptr<Chunk> fetch(int x, int z);
    std::shared_ptr<Chunk> create(int x, int z);

    void pinChunk(std::shared_ptr<Chunk> chunk);
    void unpinChunk(int x, int z);

    voxel* get(int x, int y, int z) const;

    size_t size() const;

    void incref(Chunk* chunk);
    void decref(Chunk* chunk);

    void erase(int x, int z);

    void save(Chunk* chunk);
    void saveAll();
};
