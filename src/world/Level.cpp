#include "Level.h"
#include "World.h"
#include "LevelEvents.h"
#include "../content/Content.h"
#include "../lighting/Lighting.h"
#include "../voxels/Chunk.h"
#include "../voxels/ChunksStorage.h"
#include "../physics/PhysicsSolver.h"
#include "../objects/Player.h"
#include "../items/Inventory.h"
#include "../items/Inventories.h"

#include "../voxels/ChunksMatrix.h"
Level::Level(World* world, const Content* content, EngineSettings& settings)
  : world(world),
	content(content),
	chunksStorage(std::make_unique<ChunksStorage>(this)),
	physics(std::make_unique<PhysicsSolver>(glm::vec3(0, -22.6f, 0))),
	settings(settings)
{
	auto inv = std::make_shared<Inventory>(world->getNextInventoryId(), DEF_PLAYER_INVENTORY_SIZE);
	auto player = spawnObject<Player>(this, glm::vec3(0, DEF_PLAYER_Y, 0), DEF_PLAYER_SPEED, inv, settings);

	lighting = std::make_unique<Lighting>(content, player->chunksMatrix.get(), chunksStorage.get());

	inventories = std::make_unique<Inventories>(*this);
	inventories->store(player->getInventory());
}

Level::~Level(){
	for(auto obj : objects) {
		obj.reset();
	}
}

World* Level::getWorld() {
    return world.get();
}
