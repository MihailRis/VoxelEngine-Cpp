#ifndef LOGIC_BLOCKS_CONTROLLER_H_
#define LOGIC_BLOCKS_CONTROLLER_H_

#include "../typedefs.h"

class Player;
class Block;
class Level;
class Chunks;
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
};

class BlocksController {
    Level* level;
	Chunks* chunks;
	Lighting* lighting;
    Clock randTickClock;
    uint padding;
public:
    BlocksController(Level* level, uint padding);

    void updateSides(int x, int y, int z);
    void updateBlock(int x, int y, int z);

    void breakBlock(Player* player, const Block* def, int x, int y, int z);

    void update(float delta);
    void randomTick(int tickid, int parts);
};

#endif // LOGIC_BLOCKS_CONTROLLER_H_
