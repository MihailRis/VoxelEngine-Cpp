#ifndef LOGIC_BLOCKS_CONTROLLER_H_
#define LOGIC_BLOCKS_CONTROLLER_H_

#include "../typedefs.h"
#include "../maths/fastmaths.h"

class Player;
class Block;
class Level;
class ChunksStorage;
class Lighting;

class Clock {
    int tickRate;
    int tickParts;

    float tickTimer = 0.0f;
    int tickId = 0;
    int tickPartsUndone = 0;
public:
    Clock(int tickRate, int tickParts);

    bool update(float delta);

    int getParts() const;
    int getPart() const;
    int getTickRate() const;
    int getTickId() const;
};

/* BlocksController manages block updates and block data (aka inventories) */
class BlocksController {
    Level* level;
	ChunksStorage* chunksStorage;
	Lighting* lighting;
    Clock randTickClock;
    Clock blocksTickClock;
    Clock worldTickClock;
    FastRandom random;
public:
    BlocksController(Level* level);

    void updateSides(int x, int y, int z);
    void updateBlock(int x, int y, int z);

    void breakBlock(Player* player, const Block* def, int x, int y, int z);

    void update(float delta);
    void randomTick(int tickid, int parts);
    void onBlocksTick(int tickid, int parts);
    int64_t createBlockInventory(int x, int y, int z);
    void bindInventory(int64_t invid, int x, int y, int z);
    void unbindInventory(int x, int y, int z);
};

#endif // LOGIC_BLOCKS_CONTROLLER_H_
