#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "interfaces/Serializable.hpp"
#include "voxels/voxel.hpp"

inline constexpr int STRUCTURE_FORMAT_VERSION = 1;

class Level;

struct Structure : public Serializable {
    glm::ivec3 size;
    std::vector<voxel> voxels;

    Structure() : size() {}

    Structure(glm::ivec3 size, std::vector<voxel> voxels)
    : size(size), voxels(std::move(voxels)) {}

    std::unique_ptr<dynamic::Map> serialize() const override;
    void deserialize(dynamic::Map* src) override;

    static std::unique_ptr<Structure> create(
        Level* level, const glm::ivec3& a, const glm::ivec3& b, bool entities);
};
