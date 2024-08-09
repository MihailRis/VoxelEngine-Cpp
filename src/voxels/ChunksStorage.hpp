#ifndef VOXELS_CHUNKSSTORAGE_HPP_
#define VOXELS_CHUNKSSTORAGE_HPP_

#include <memory>
#include <unordered_map>

#include "typedefs.hpp"
#include "voxel.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class Chunk;
class Level;
class VoxelsVolume;

class ChunksStorage {
    Level* level;
    std::unordered_map<glm::ivec2, std::shared_ptr<Chunk>> chunksMap;
public:
    ChunksStorage(Level* level);
    ~ChunksStorage() = default;

    std::shared_ptr<Chunk> get(int x, int z) const;
    void store(const std::shared_ptr<Chunk>& chunk);
    void remove(int x, int y);
    void getVoxels(VoxelsVolume* volume, bool backlight = false) const;
    std::shared_ptr<Chunk> create(int x, int z);
};

#endif  // VOXELS_CHUNKSSTORAGE_HPP_
