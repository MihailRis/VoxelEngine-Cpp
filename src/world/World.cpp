#include "World.h"

#include <memory>
#include <glm/glm.hpp>

#include "Level.h"
#include "../files/WorldFiles.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksStorage.h"
#include "../objects/Player.h"
#include "../window/Camera.h"

using glm::vec3;
using std::shared_ptr;
using std::string;
using std::filesystem::path;

World::World(string name, 
			 path directory, 
			 uint64_t seed, 
			 EngineSettings& settings) 
			: name(name), seed(seed) {
	wfile = new WorldFiles(directory, settings.debug.generatorTestMode);
}

World::~World(){
	delete wfile;
}

void World::write(Level* level, bool writeChunks) {
	Chunks* chunks = level->chunks;

	for (size_t i = 0; i < chunks->volume; i++) {
		shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr || !chunk->isUnsaved())
			continue;
		wfile->put(chunk.get());
	}

	wfile->write(WorldInfo {name, wfile->directory, seed});
	wfile->writePlayer(level->player);
}

Level* World::load(EngineSettings& settings) {
	WorldInfo info {name, wfile->directory, seed};
	wfile->readWorldInfo(info);
	seed = info.seed;
	name = info.name;

	vec3 playerPosition = vec3(0, 64, 0);
	Camera* camera = new Camera(playerPosition, glm::radians(90.0f));
	Player* player = new Player(playerPosition, 4.0f, camera);
	Level* level = new Level(this, player, settings);
	wfile->readPlayer(player);

	camera->rotation = mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);
	return level;
}