#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "interfaces/Serializable.hpp"
#include "voxels/voxel.hpp"

inline constexpr int STRUCTURE_FORMAT_VERSION = 1;

class Level;
class Content;

struct VoxelStructure : public Serializable {
    glm::ivec3 size;

    /// @brief Structure voxels indexed different to world content
    std::vector<voxel> voxels;
    /// @brief Block names are used for indexing
    std::vector<std::string> blockNames;

    VoxelStructure() : size() {}

    VoxelStructure(
        glm::ivec3 size,
        std::vector<voxel> voxels,
        std::vector<std::string> blockNames
    ):  size(size), 
        voxels(std::move(voxels)), 
        blockNames(std::move(blockNames)) 
    {}

    std::unique_ptr<dynamic::Map> serialize() const override;
    void deserialize(dynamic::Map* src) override;

    static std::unique_ptr<VoxelStructure> create(
        Level* level, const glm::ivec3& a, const glm::ivec3& b, bool entities);
};
