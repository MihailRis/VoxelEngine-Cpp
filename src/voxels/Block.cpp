#include "Block.hpp"

#include <utility>

#include "../core_defs.hpp"
#include "../util/stringutil.hpp"

std::string to_string(BlockModel model) {
    switch (model) {
        case BlockModel::none:
            return "none";
        case BlockModel::block:
            return "block";
        case BlockModel::xsprite:
            return "X";
        case BlockModel::aabb:
            return "aabb";
        case BlockModel::custom:
            return "custom";
        default:
            return "unknown";
    }
}

std::optional<BlockModel> BlockModel_from(std::string_view str) {
    if (str == "none") {
        return BlockModel::none;
    } else if (str == "block") {
        return BlockModel::block;
    } else if (str == "X") {
        return BlockModel::xsprite;
    } else if (str == "aabb") {
        return BlockModel::aabb;
    } else if (str == "custom") {
        return BlockModel::custom;
    }
    return std::nullopt;
}

CoordSystem::CoordSystem(glm::ivec3 axisX, glm::ivec3 axisY, glm::ivec3 axisZ)
    : axisX(axisX), axisY(axisY), axisZ(axisZ) {
    fix = glm::ivec3(0);
    if (isVectorHasNegatives(axisX)) fix -= axisX;
    if (isVectorHasNegatives(axisY)) fix -= axisY;
    if (isVectorHasNegatives(axisZ)) fix -= axisZ;
}

void CoordSystem::transform(AABB& aabb) const {
    glm::vec3 X(axisX);
    glm::vec3 Y(axisY);
    glm::vec3 Z(axisZ);
    aabb.a = X * aabb.a.x + Y * aabb.a.y + Z * aabb.a.z;
    aabb.b = X * aabb.b.x + Y * aabb.b.y + Z * aabb.b.z;
    aabb.a += fix;
    aabb.b += fix;
}

const BlockRotProfile BlockRotProfile::NONE {
    "none",
    {
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},  // North
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},  // East
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},  // South
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},  // West
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},  // Up
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},  // Down
    }};

const BlockRotProfile BlockRotProfile::PIPE {
    "pipe",
    {
        {{1, 0, 0}, {0, 0, 1}, {0, -1, 0}},    // North
        {{0, 0, 1}, {-1, 0, 0}, {0, -1, 0}},   // East
        {{-1, 0, 0}, {0, 0, -1}, {0, -1, 0}},  // South
        {{0, 0, -1}, {1, 0, 0}, {0, -1, 0}},   // West
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},     // Up
        {{1, 0, 0}, {0, -1, 0}, {0, 0, -1}},   // Down
    }};

const BlockRotProfile BlockRotProfile::PANE {
    "pane",
    {
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},    // North
        {{0, 0, -1}, {0, 1, 0}, {1, 0, 0}},   // East
        {{-1, 0, 0}, {0, 1, 0}, {0, 0, -1}},  // South
        {{0, 0, 1}, {0, 1, 0}, {-1, 0, 0}},   // West
    }};

Block::Block(const std::string& name)
    : name(name),
      caption(util::id_to_caption(name)),
      textureFaces {
          TEXTURE_NOTFOUND,
          TEXTURE_NOTFOUND,
          TEXTURE_NOTFOUND,
          TEXTURE_NOTFOUND,
          TEXTURE_NOTFOUND,
          TEXTURE_NOTFOUND} {
}

Block::Block(std::string name, const std::string& texture)
    : name(std::move(name)),
      textureFaces {texture, texture, texture, texture, texture, texture} {
}
