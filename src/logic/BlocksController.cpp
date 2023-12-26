#include "BlocksController.h"

#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../world/Level.h"
#include "../content/Content.h"
#include "../lighting/Lighting.h"
#include "../util/timeutil.h"

#include "scripting/scripting.h"

Clock::Clock(int tickRate, int tickParts)
    : tickRate(tickRate),
      tickParts(tickParts) {
}

bool Clock::update(float delta) {
    tickTimer += delta;
    float delay = 1.0f / float(tickRate);    
    if (tickTimer > delay || tickPartsUndone) {
        if (tickPartsUndone) {
            tickPartsUndone--;
        } else {
            tickTimer = fmod(tickTimer, delay);
            tickPartsUndone = tickParts-1;
        }
        return true;
    }
    return false;
}

int Clock::getParts() const {
    return tickParts;
}

int Clock::getPart() const {
    return tickParts-tickPartsUndone-1;
}

BlocksController::BlocksController(Level* level, uint padding) 
    : level(level), 
	  chunks(level->chunks), 
	  lighting(level->lighting),
      randTickClock(20, 3),
      padding(padding) {
}

void BlocksController::updateSides(int x, int y, int z) {
    updateBlock(x-1, y, z);
    updateBlock(x+1, y, z);
    updateBlock(x, y-1, z);
    updateBlock(x, y+1, z);
    updateBlock(x, y, z-1);
    updateBlock(x, y, z+1);
}

void BlocksController::breakBlock(Player* player, const Block* def, int x, int y, int z) {
    chunks->set(x,y,z, 0, 0);
    lighting->onBlockSet(x,y,z, 0);
    if (def->rt.funcsset.onbroken) {
        scripting::on_block_broken(player, def, x, y, z);
    }
    updateSides(x, y, z);
}

void BlocksController::updateBlock(int x, int y, int z) {
    voxel* vox = chunks->get(x, y, z);
    if (vox == nullptr)
        return;
    const Block* def = level->content->indices->getBlockDef(vox->id);
    if (def->grounded && !chunks->isSolid(x, y-1, z)) {
        breakBlock(nullptr, def, x, y, z);
        return;
    }
    if (def->rt.funcsset.update) {
        scripting::update_block(def, x, y, z);
    }
}

void BlocksController::update(float delta) {
    if (randTickClock.update(delta)) {
        randomTick(randTickClock.getPart(), randTickClock.getParts());
    }
}

void BlocksController::randomTick(int tickid, int parts) {
    // timeutil::ScopeLogTimer timer(5000+tickid);
    const int w = chunks->w;
    const int d = chunks->d;
    auto indices = level->content->indices;
    for (uint z = padding; z < d-padding; z++){
        for (uint x = padding; x < w-padding; x++){
            int index = z * w + x;
            if ((index + tickid) % parts != 0)
                continue;
            std::shared_ptr<Chunk> chunk = chunks->chunks[index];
            if (chunk == nullptr || !chunk->isLighted())
                continue;
            int segments = 4;
            int segheight = CHUNK_H / segments;
            for (int s = 0; s < segments; s++) {
                for (int i = 0; i < 3; i++) {
                    int bx = rand() % CHUNK_W;
                    int by = rand() % segheight + s * segheight;
                    int bz = rand() % CHUNK_D;
                    const voxel& vox = chunk->voxels[(by * CHUNK_D + bz) * CHUNK_W + bx];
                    Block* block = indices->getBlockDef(vox.id);
                    if (block->rt.funcsset.randupdate) {
                        scripting::random_update_block(
                            block, 
                            chunk->x * CHUNK_W + bx, by, 
                            chunk->z * CHUNK_D + bz);
                    }
                }
            }
        }
	}
}
