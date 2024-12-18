#pragma once

#include <functional>
#include <glm/glm.hpp>

#include "maths/fastmaths.hpp"
#include "typedefs.hpp"
#include "util/Clock.hpp"
#include "voxels/voxel.hpp"

class Player;
class Block;
class Level;
class Chunk;
class Chunks;
class Lighting;
class GlobalChunks;
class ContentIndices;

enum class BlockInteraction { step, destruction, placing };

/// @brief Player argument is nullable
using on_block_interaction = std::function<
    void(Player*, const glm::ivec3&, const Block&, BlockInteraction)>;

/// BlocksController manages block updates and data (inventories, metadata)
class BlocksController {
    const Level& level;
    GlobalChunks& chunks;
    Lighting* lighting;
    util::Clock randTickClock;
    util::Clock blocksTickClock;
    util::Clock worldTickClock;
    FastRandom random {};
    std::vector<on_block_interaction> blockInteractionCallbacks;
public:
    BlocksController(const Level& level, Lighting* lighting);

    void updateSides(int x, int y, int z);
    void updateSides(int x, int y, int z, int w, int h, int d);
    void updateBlock(int x, int y, int z);

    void breakBlock(Player* player, const Block& def, int x, int y, int z);
    void placeBlock(
        Player* player, const Block& def, blockstate state, int x, int y, int z
    );

    void update(float delta, uint padding);
    void randomTick(
        const Chunk& chunk, int segments, const ContentIndices* indices
    );
    void randomTick(int tickid, int parts, uint padding);
    void onBlocksTick(int tickid, int parts);
    int64_t createBlockInventory(int x, int y, int z);
    void bindInventory(int64_t invid, int x, int y, int z);
    void unbindInventory(int x, int y, int z);

    void onBlockInteraction(
        Player* player, glm::ivec3 pos, const Block& def, BlockInteraction type
    );

    /// @brief Add block interaction callback
    void listenBlockInteraction(const on_block_interaction& callback);
};
