#include "Level.h"
#include "../lighting/Lighting.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksController.h"
#include "../player_control.h"
#include "../physics/PhysicsSolver.h"
#include "../objects/Player.h"

Level::Level(World* world, Player* player, Chunks* chunks, PhysicsSolver* physics) :
	world(world),
	player(player),
	chunks(chunks),
	physics(physics) {
	lighting = new Lighting(chunks);
	chunksController = new ChunksController(world, chunks, lighting);
	playerController = new PlayerController(this);
}

Level::~Level(){
	delete chunks;
	delete physics;
	delete player;
	delete lighting;
	delete chunksController;
	delete playerController;
}
