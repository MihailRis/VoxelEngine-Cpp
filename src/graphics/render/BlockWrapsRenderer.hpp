#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "MainBatch.hpp"
#include "typedefs.hpp"

class Assets;
class Player;
class Level;
class Chunks;
class DrawContext;

struct BlockWrapper {
    glm::ivec3 position;
    std::string texture;
};

class BlockWrapsRenderer {
    const Assets& assets;
    const Level& level;
    const Chunks& chunks;
    std::unique_ptr<MainBatch> batch;

    std::unordered_map<u64id_t, std::unique_ptr<BlockWrapper>> wrappers;
    u64id_t nextWrapper = 1;

    void draw(const BlockWrapper& wrapper);
public:
    BlockWrapsRenderer(
        const Assets& assets, const Level& level, const Chunks& chunks
    );
    ~BlockWrapsRenderer();

    void draw(const DrawContext& ctx, const Player& player);

    u64id_t add(const glm::ivec3& position, const std::string& texture);

    BlockWrapper* get(u64id_t id) const;

    void remove(u64id_t id);
};
