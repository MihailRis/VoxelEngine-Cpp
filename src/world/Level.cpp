#include "Level.h"
#include "World.h"
#include "LevelEvents.h"
#include "../lighting/Lighting.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksController.h"
#include "../voxels/ChunksStorage.h"
#include "../physics/Hitbox.h"
#include "../physics/PhysicsSolver.h"
#include "../objects/Player.h"
#include "../objects/player_control.h"

Level::Level(World* world, Player* player, ChunksStorage* chunksStorage, LevelEvents* events, uint loadDistance, uint chunksPadding) :
		world(world),
		player(player),
		chunksStorage(chunksStorage),
		events(events) {
    physics = new PhysicsSolver(vec3(0, -19.6f, 0));
    uint matrixSize = (loadDistance+chunksPadding) * 2;
    chunks = new Chunks(matrixSize, matrixSize, 0, 0, world->wfile, events);
	lighting = new Lighting(chunks);
	chunksController = new ChunksController(this, chunks, lighting, chunksPadding);
	playerController = new PlayerController(this);
	events->listen(EVT_CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		this->chunksStorage->remove(chunk->x, chunk->z);
	});
}

Level::~Level(){
	delete chunks;
	delete events;
	delete physics;
	delete player;
	delete lighting;
	delete chunksController;
	delete chunksStorage;
	delete playerController;
}

void Level::update(float delta, bool updatePlayer, bool interactions) {
	playerController->update_controls(delta, updatePlayer);
	if (interactions) {
		playerController->update_interaction();
	}
	else
	{
		playerController->selectedBlockId = -1;
	}
	vec3 position = player->hitbox->position;
	chunks->setCenter(position.x, position.z);
}
