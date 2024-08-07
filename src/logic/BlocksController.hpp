#ifndef LOGIC_BLOCKS_CONTROLLER_HPP_
#define LOGIC_BLOCKS_CONTROLLER_HPP_

#include <functional>
#include <glm/glm.hpp>

#include <maths/fastmaths.hpp>
#include <typedefs.hpp>
#include <util/Clock.hpp>
#include <voxels/voxel.hpp>

class Player;
class Block;
class Level;
class Chunk;
class Chunks;
class Lighting;
class ContentIndices;

enum class BlockInteraction { step, destruction, placing };

/// @brief Player argument is nullable
using on_block_interaction = std::function<
    void(Player*, glm::ivec3, const Block&, BlockInteraction type)>;

/// BlocksController manages block updates and data (inventories, metadata)
class BlocksController {
    Level* level;
    Chunks* chunks;
    Lighting* lighting;
    util::Clock randTickClock;
    util::Clock blocksTickClock;
    util::Clock worldTickClock;
    uint padding;
    FastRandom random;
    std::vector<on_block_interaction> blockInteractionCallbacks;
public:
    BlocksController(Level* level, uint padding);

    void updateSides(int x, int y, int z);
    void updateBlock(int x, int y, int z);

    void breakBlock(Player* player, const Block& def, int x, int y, int z);
    void placeBlock(
        Player* player, const Block& def, blockstate state, int x, int y, int z
    );

    void update(float delta);
    void randomTick(
        const Chunk& chunk, int segments, const ContentIndices* indices
    );
    void randomTick(int tickid, int parts);
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

#endif  // LOGIC_BLOCKS_CONTROLLER_HPP_
