#include "Level.h"
#include "World.h"
#include "../lighting/Lighting.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksController.h"
#include "../voxels/ChunksStorage.h"
#include "../player_control.h"
#include "../physics/Hitbox.h"
#include "../physics/PhysicsSolver.h"
#include "../objects/Player.h"

Level::Level(World* world, Player* player, Chunks* chunks, ChunksStorage* chunksStorage, PhysicsSolver* physics) :
	world(world),
	player(player),
	chunks(chunks),
	chunksStorage(chunksStorage),
	physics(physics) {
	lighting = new Lighting(chunks);
	chunksController = new ChunksController(this, chunks, lighting);
	playerController = new PlayerController(this);
}

Level::~Level(){
	delete chunks;
	delete physics;
	delete player;
	delete lighting;
	delete chunksController;
	delete chunksStorage;
	delete playerController;
}

void Level::update(float delta, bool interactions) {
	playerController->update_controls(delta);
	if (interactions) {
		playerController->update_interaction();
	}
	else
	{
		playerController->selectedBlockId = -1;
	}
	vec3 position = player->hitbox->position;
	chunks->setCenter(world->wfile, position.x, position.z);
}
