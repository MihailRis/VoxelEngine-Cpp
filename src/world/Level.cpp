#include "Level.h"
#include "World.h"
#include "LevelEvents.h"
#include "../content/Content.h"
#include "../lighting/Lighting.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksStorage.h"
#include "../physics/Hitbox.h"
#include "../physics/PhysicsSolver.h"
#include "../objects/Player.h"
#include "../items/Inventory.h"
#include "../items/Inventories.h"

Level::Level(World* world, const Content* content, EngineSettings& settings)
	  : world(world),
	    content(content),
		chunksStorage(std::make_unique<ChunksStorage>(this)),
		physics(std::make_unique<PhysicsSolver>(glm::vec3(0, -22.6f, 0))),
        events(std::make_unique<LevelEvents>()),
		settings(settings) 
{
	auto inv = std::make_shared<Inventory>(world->getNextInventoryId(), DEF_PLAYER_INVENTORY_SIZE);
	auto player = spawnObject<Player>(glm::vec3(0, DEF_PLAYER_Y, 0), DEF_PLAYER_SPEED, inv);

    uint matrixSize = (settings.chunks.loadDistance + settings.chunks.padding) * 2;
    chunks = std::make_unique<Chunks>(matrixSize, matrixSize, 0, 0, 
						world->wfile.get(), events.get(), content);
	lighting = std::make_unique<Lighting>(content, chunks.get());

	events->listen(EVT_CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		this->chunksStorage->remove(chunk->x, chunk->z);
	});

	inventories = std::make_unique<Inventories>(*this);
	inventories->store(player->getInventory());
}

Level::~Level(){
	for(auto obj : objects) {
		obj.reset();
	}
}

void Level::loadMatrix(int32_t x, int32_t z, uint32_t radius) {
	chunks->setCenter(x, z);
    uint32_t diameter = std::min(radius*2, (settings.chunks.loadDistance + settings.chunks.padding) * 2);
	if (chunks->w != diameter) {
		chunks->resize(diameter, diameter);
	}
}

World* Level::getWorld() {
    return world.get();
}

