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

Level::Level(World* world, const Content* content, Player* player, EngineSettings& settings)
	  : world(world),
	    content(content),
		player(player),
		chunksStorage(new ChunksStorage(this)),
		events(new LevelEvents()) ,
		settings(settings) {
    physics = new PhysicsSolver(glm::vec3(0, -22.6f, 0));

    uint matrixSize = (settings.chunks.loadDistance+
					   settings.chunks.padding) * 2;
    chunks = new Chunks(matrixSize, matrixSize, 0, 0, 
						world->wfile, events, content);
	lighting = new Lighting(content, chunks);

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
	delete chunksStorage;
}

void Level::update() {
	glm::vec3 position = player->hitbox->position;
	chunks->setCenter(position.x, position.z);

	int matrixSize = (settings.chunks.loadDistance+
					  settings.chunks.padding) * 2;
	if (chunks->w != matrixSize) {
		chunks->resize(matrixSize, matrixSize);
	}
}

World* Level::getWorld() {
    return world.get();
}
