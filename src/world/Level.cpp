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

Level::Level(World* world, Player* player, ChunksStorage* chunksStorage, LevelEvents* events, EngineSettings& settings) :
		world(world),
		player(player),
		chunksStorage(chunksStorage),
		events(events) {
    physics = new PhysicsSolver(vec3(0, -19.6f, 0));

    uint matrixSize = (settings.chunks.loadDistance+
					   settings.chunks.padding) * 2;
    chunks = new Chunks(matrixSize, matrixSize, 0, 0, world->wfile, events);
	lighting = new Lighting(chunks);
	chunksController = new ChunksController(this, chunks, lighting, settings.chunks.padding);
	playerController = new PlayerController(this, settings);

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

void Level::updatePlayer(float delta, 
				  		 bool input, 
				 	     bool pause, 
				  		 bool interactions) {
	if (!pause) {
		if (input) {
			playerController->updateKeyboard();
			playerController->updateCameraControl();
		} else {
			playerController->resetKeyboard();
		}
		playerController->updateControls(delta);

	}
	playerController->refreshCamera();
	if (interactions) {
		playerController->updateInteraction();
	} else {
		playerController->selectedBlockId = -1;
	}
}

void Level::update() {
	vec3 position = player->hitbox->position;
	chunks->setCenter(position.x, position.z);
}
