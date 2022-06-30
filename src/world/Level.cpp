#include "Level.h"
#include "../lighting/Lighting.h"
#include "../voxels/ChunksController.h"

Level::Level(World* world, Player* player, Chunks* chunks, PhysicsSolver* physics) :
	player(player),
	chunks(chunks),
	physics(physics) {
	lighting = new Lighting(chunks);
	chunksController = new ChunksController(world, chunks, lighting);
}

Level::~Level(){
	delete chunks;
	delete physics;
	delete player;
	delete lighting;
	delete chunksController;
}
