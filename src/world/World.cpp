#include "World.h"

#include <memory>
#include <iostream>
#include <glm/glm.hpp>

#include "Level.h"
#include "../files/WorldFiles.h"
#include "../content/Content.h"
#include "../content/ContentLUT.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksStorage.h"
#include "../objects/Player.h"
#include "../window/Camera.h"

world_load_error::world_load_error(std::string message) 
    : std::runtime_error(message) {
}

World::World(
    std::string name, 
    fs::path directory, 
    uint64_t seed, 
    EngineSettings& settings,
    const Content* content,
    const std::vector<ContentPack> packs) 
    : name(name),
      seed(seed),
      settings(settings), 
      content(content),
      packs(packs) {
    wfile = new WorldFiles(directory, settings.debug);
}

World::~World(){
    delete wfile;
}

void World::updateTimers(float delta) {
    daytime += delta * daytimeSpeed;
    daytime = fmod(daytime, 1.0f);
    totalTime += delta;
}

void World::write(Level* level) {
    const Content* content = level->content;

    Chunks* chunks = level->chunks;

    for (size_t i = 0; i < chunks->volume; i++) {
        auto chunk = chunks->chunks[i];
        if (chunk == nullptr || !chunk->isLighted())
            continue;
        bool lightsUnsaved = !chunk->isLoadedLights() && 
                              settings.debug.doWriteLights;
        if (!chunk->isUnsaved() && !lightsUnsaved)
            continue;
        wfile->put(chunk.get());
    }

    wfile->write(this, content);
    wfile->writePlayer(level->player);
}

Level* World::create(std::string name, 
                     fs::path directory, 
                     uint64_t seed,
                     EngineSettings& settings, 
                     const Content* content,
                     const std::vector<ContentPack>& packs) {

    auto world = new World(name, directory, seed, settings, content, packs);
    return new Level(world, content, settings);
}

Level* World::load(fs::path directory,
                   EngineSettings& settings,
                   const Content* content,
                   const std::vector<ContentPack>& packs) {
    auto world = std::make_unique<World>(
        ".", directory, 0, settings, content, packs
    );
    auto& wfile = world->wfile;

    if (!wfile->readWorldInfo(world.get())) {
        throw world_load_error("could not to find world.json");
    }


    auto level = new Level(world.get(), content, settings);
    wfile->readPlayer(level->player);

    world.release();
    return level;
}

ContentLUT* World::checkIndices(const fs::path& directory, 
                                const Content* content) {
    fs::path indicesFile = directory/fs::path("indices.json");
    if (fs::is_regular_file(indicesFile)) {
        return ContentLUT::create(indicesFile, content);
    }
    return nullptr;
}

void World::setName(const std::string& name) {
    this->name = name;
}

bool World::hasPack(const std::string& id) const {
    for (auto& pack : packs) {
        if (pack.id == id)
            return true;
    }
    return false;
}

void World::setSeed(uint64_t seed) {
    this->seed = seed;
}

std::string World::getName() const {
    return name;
}

uint64_t World::getSeed() const {
    return seed;
}

const std::vector<ContentPack>& World::getPacks() const {
    return packs;
}

void World::deserialize(dynamic::Map* root) {
    name = root->getStr("name", name);
    seed = root->getInt("seed", seed);

	auto verobj = root->map("version");
	if (verobj) {
		int major=0, minor=-1;
		verobj->num("major", major);
		verobj->num("minor", minor);
		std::cout << "world version: " << major << "." << minor << std::endl;
	}

	auto timeobj = root->map("time");
	if (timeobj) {
		timeobj->num("day-time", daytime);
		timeobj->num("day-time-speed", daytimeSpeed);
        timeobj->num("total-time", totalTime);
	}
    
    nextInventoryId = root->getNum("next-inventory-id", 2);
}

std::unique_ptr<dynamic::Map> World::serialize() const {
	auto root = std::make_unique<dynamic::Map>();

	auto& versionobj = root->putMap("version");
	versionobj.put("major", ENGINE_VERSION_MAJOR);
	versionobj.put("minor", ENGINE_VERSION_MINOR);

	root->put("name", name);
	root->put("seed", seed);
	
    auto& timeobj = root->putMap("time");
	timeobj.put("day-time", daytime);
	timeobj.put("day-time-speed", daytimeSpeed);
    timeobj.put("total-time", totalTime);

    root->put("next-inventory-id", nextInventoryId);
    return root;
}
