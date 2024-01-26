#include "World.h"

#include <memory>
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

const float DEF_PLAYER_Y = 100.0f;
const float DEF_PLAYER_SPEED = 4.0f;

Level* World::create(std::string name, 
                     fs::path directory, 
                     uint64_t seed,
                     EngineSettings& settings, 
                     const Content* content,
                     const std::vector<ContentPack>& packs) {
    World* world = new World(name, directory, seed, settings, content, packs);
    Player* player = new Player(glm::vec3(0, DEF_PLAYER_Y, 0), DEF_PLAYER_SPEED);
    return new Level(world, content, player, settings);
}

ContentLUT* World::checkIndices(const fs::path& directory, 
                                const Content* content) {
    fs::path indicesFile = directory/fs::path("indices.json");
    if (fs::is_regular_file(indicesFile)) {
        return ContentLUT::create(indicesFile, content);
    }
    return nullptr;
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

    Player* player = new Player(glm::vec3(0, DEF_PLAYER_Y, 0), DEF_PLAYER_SPEED);
    Level* level = new Level(world.get(), content, player, settings);
    wfile->readPlayer(player);

    world.release();
    return level;
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
