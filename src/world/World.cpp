#include "World.h"

#include <memory>
#include <glm/glm.hpp>

#include "Level.h"
#include "../files/WorldFiles.h"
#include "../content/Content.h"
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
		: settings(settings), 
		  name(name), 
		  seed(seed) {
	wfile = new WorldFiles(directory, settings.debug);
}

World::~World(){
	delete wfile;
}

void World::updateTimers(float delta) {
	daytime += delta * daytimeSpeed;
	daytime = fmod(daytime, 1.0f);
}

void World::write(Level* level) {
	const Content* content = level->content;

	Chunks* chunks = level->chunks;

	for (size_t i = 0; i < chunks->volume; i++) {
		shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr)
			continue;
		bool lightsUnsaved = !chunk->isLoadedLights() && settings.debug.doWriteLights;
		if (!chunk->isUnsaved() && !lightsUnsaved)
			continue;
		wfile->put(chunk.get());
	}

	wfile->write(this, content);
	wfile->writePlayer(level->player);
}

Level* World::load(EngineSettings& settings, const Content* content) {
	wfile->readWorldInfo(this);

	vec3 playerPosition = vec3(0, 100, 0);
	Camera* camera = new Camera(playerPosition, glm::radians(90.0f));
	Player* player = new Player(playerPosition, 4.0f, camera);
	Level* level = new Level(this, content, player, settings);

	wfile->readPlayer(player);

	camera->rotation = glm::mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);
	return level;
}