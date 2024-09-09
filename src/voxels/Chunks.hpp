#pragma once

#include <stdlib.h>

#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <vector>

#include "typedefs.hpp"
#include "voxel.hpp"
#include "util/AreaMap2D.hpp"

class VoxelRenderer;

struct AABB;
class Content;
class ContentIndices;
class Chunk;
class WorldFiles;
class LevelEvents;
class Block;
class Level;

/// Player-centred chunks matrix
class Chunks {
    Level* level;
    const ContentIndices* const indices;

    void eraseSegments(const Block& def, blockstate state, int x, int y, int z);
    void repairSegments(
        const Block& def, blockstate state, int x, int y, int z
    );
    void setRotationExtended(
        const Block& def, blockstate state, glm::ivec3 origin, uint8_t rotation
    );

    util::AreaMap2D<std::shared_ptr<Chunk>, int32_t> areaMap;
public:
    size_t visible = 0;
    WorldFiles* worldFiles;

    Chunks(
        int32_t w,
        int32_t d,
        int32_t ox,
        int32_t oz,
        WorldFiles* worldFiles,
        Level* level
    );
    ~Chunks() = default;

    bool putChunk(const std::shared_ptr<Chunk>& chunk);

    Chunk* getChunk(int32_t x, int32_t z);
    Chunk* getChunkByVoxel(int32_t x, int32_t y, int32_t z);
    voxel* get(int32_t x, int32_t y, int32_t z) const;

    inline voxel* get(glm::ivec3 pos) {
        return get(pos.x, pos.y, pos.z);
    }

    light_t getLight(int32_t x, int32_t y, int32_t z);
    ubyte getLight(int32_t x, int32_t y, int32_t z, int channel);
    void set(int32_t x, int32_t y, int32_t z, uint32_t id, blockstate state);

    /// @brief Seek for the extended block origin position
    /// @param pos segment block position
    /// @param def segment block definition
    /// @param state segment block state
    /// @return origin block position or `pos` if block is not extended
    glm::ivec3 seekOrigin(glm::ivec3 pos, const Block& def, blockstate state);

    /// @brief Check if required zone is replaceable
    /// @param def definition of the block that requires a replaceable zone
    /// @param state the block state
    /// @param coord position of the zone start
    /// @param ignore ignored block id (will be counted as replaceable)
    bool checkReplaceability(
        const Block& def,
        blockstate state,
        glm::ivec3 coord,
        blockid_t ignore = 0
    );

    void setRotation(int32_t x, int32_t y, int32_t z, uint8_t rotation);

    voxel* rayCast(
        glm::vec3 start,
        glm::vec3 dir,
        float maxLength,
        glm::vec3& end,
        glm::ivec3& norm,
        glm::ivec3& iend,
        std::set<blockid_t> filter = {}
    );

    glm::vec3 rayCastToObstacle(glm::vec3 start, glm::vec3 dir, float maxDist);

    const AABB* isObstacleAt(float x, float y, float z);
    bool isSolidBlock(int32_t x, int32_t y, int32_t z);
    bool isReplaceableBlock(int32_t x, int32_t y, int32_t z);
    bool isObstacleBlock(int32_t x, int32_t y, int32_t z);

    void setCenter(int32_t x, int32_t z);
    void resize(uint32_t newW, uint32_t newD);

    void saveAndClear();
    void save(Chunk* chunk);
    void saveAll();

    const std::vector<std::shared_ptr<Chunk>>& getChunks() const {
        return areaMap.getBuffer();
    }

    int getWidth() const {
        return areaMap.getWidth();
    }

    int getHeight() const {
        return areaMap.getHeight();
    }

    int getOffsetX() const {
        return areaMap.getOffsetX();
    }

    int getOffsetY() const {
        return areaMap.getOffsetY();
    }

    size_t getChunksCount() const {
        return areaMap.count();
    }

    size_t getVolume() const {
        return areaMap.area();
    }
};
