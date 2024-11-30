#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>
#include <functional>

#include "MainBatch.hpp"
#include "typedefs.hpp"

class Assets;
class Player;
class Level;
class DrawContext;

template <>
struct std::hash<glm::ivec3> {
    size_t operator()(const glm::ivec3& vec) const noexcept {
        return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1) ^ (std::hash<int>()(vec.z) << 2);
    }
};

struct BlockWrapper {
    glm::ivec3 position;
    std::string texture;
};

class BlockWrapsRenderer {
    const Assets& assets;
    const Level& level;
    std::unique_ptr<MainBatch> batch;

    std::unordered_map<u64id_t, std::unique_ptr<BlockWrapper>> wrappers;
    std::unordered_map<glm::ivec3, std::unordered_set<u64id_t>> positionIndex;
    u64id_t nextWrapper = 1;

    void draw(const BlockWrapper& wrapper);
public:
    BlockWrapsRenderer(const Assets& assets, const Level& level);
    ~BlockWrapsRenderer();

    void draw(const DrawContext& ctx, const Player& player);

    u64id_t add(const glm::ivec3& position, const std::string& texture);

    BlockWrapper* get(u64id_t id) const;
    const std::unordered_set<u64id_t>* get_ids_by_position(const glm::ivec3& position) const;

    void remove(u64id_t id);
    void remove_by_position(const glm::ivec3& position);
};
