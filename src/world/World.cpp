#include "World.hpp"

#include "Level.hpp"
#include "../content/Content.hpp"
#include "../content/ContentLUT.hpp"
#include "../files/WorldFiles.hpp"
#include "../items/Inventories.hpp"
#include "../objects/Player.hpp"
#include "../voxels/Chunk.hpp"
#include "../voxels/Chunks.hpp"
#include "../voxels/ChunksStorage.hpp"
#include "../window/Camera.hpp"
#include "../world/WorldGenerators.hpp"

#include <memory>
#include <iostream>
#include <glm/glm.hpp>

world_load_error::world_load_error(std::string message) 
    : std::runtime_error(message) {
}

World::World(
    std::string name, 
    std::string generator,
    fs::path directory, 
    uint64_t seed, 
    EngineSettings& settings,
    const Content* content,
    const std::vector<ContentPack> packs
) : name(name),
    generator(generator),
    seed(seed),
    settings(settings), 
    content(content),
    packs(packs) 
{
    wfile = std::make_unique<WorldFiles>(directory, settings.debug);
}

World::~World(){
}

void World::updateTimers(float delta) {
    daytime += delta * daytimeSpeed;
    daytime = fmod(daytime, 1.0f);
    totalTime += delta;
}

void World::write(Level* level) {
    const Content* content = level->content;
    Chunks* chunks = level->chunks.get();
    auto& regions = wfile->getRegions();

    for (size_t i = 0; i < chunks->volume; i++) {
        auto chunk = chunks->chunks[i];
        if (chunk == nullptr || !chunk->isLighted())
            continue;
        bool lightsUnsaved = !chunk->isLoadedLights() && 
                              settings.debug.doWriteLights.get();
        if (!chunk->isUnsaved() && !lightsUnsaved)
            continue;
        regions.put(chunk.get());
    }

    wfile->write(this, content);
	auto playerFile = dynamic::Map();
    {
        auto& players = playerFile.putList("players");
        for (auto object : level->objects) {
            if (std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(object)) {
                players.put(player->serialize().release());
            }
        }
    }
    files::write_json(wfile->getPlayerFile(), &playerFile);
}

Level* World::create(std::string name, 
                     std::string generator,
                     fs::path directory, 
                     uint64_t seed,
                     EngineSettings& settings, 
                     const Content* content,
                     const std::vector<ContentPack>& packs
) {
    auto world = new World(name, generator, directory, seed, settings, content, packs);
    auto level = new Level(world, content, settings);
    return level;
}

Level* World::load(fs::path directory,
                   EngineSettings& settings,
                   const Content* content,
                   const std::vector<ContentPack>& packs) {
    auto world = std::make_unique<World>(
        ".", WorldGenerators::getDefaultGeneratorID(), directory, 0, settings, content, packs
    );
    auto& wfile = world->wfile;

    if (!wfile->readWorldInfo(world.get())) {
        throw world_load_error("could not to find world.json");
    }

    auto level = new Level(world.get(), content, settings);
    {
        fs::path file = wfile->getPlayerFile();
        if (!fs::is_regular_file(file)) {
            std::cerr << "warning: player.json does not exists" << std::endl;
        } else {
            auto playerFile = files::read_json(file);
            if (playerFile->has("players")) {
                level->objects.clear();
                auto players = playerFile->list("players");
                for (size_t i = 0; i < players->size(); i++) {
                    auto player = level->spawnObject<Player>(glm::vec3(0, DEF_PLAYER_Y, 0), DEF_PLAYER_SPEED, level->inventories->create(DEF_PLAYER_INVENTORY_SIZE));
                    player->deserialize(players->map(i));
                    level->inventories->store(player->getInventory());
                }
            } else {
	            auto player = level->getObject<Player>(0);
                player->deserialize(playerFile.get());
                level->inventories->store(player->getInventory());
            }
        }
    }
    (void)world.release();
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

void World::setGenerator(const std::string& generator) {
    this->generator = generator;
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

std::string World::getGenerator() const {
    return generator;
}

const std::vector<ContentPack>& World::getPacks() const {
    return packs;
}

void World::deserialize(dynamic::Map* root) {
    name = root->get("name", name);
    generator = root->get("generator", generator);
    seed = root->get("seed", seed);

    if(generator == "") {
        generator = WorldGenerators::getDefaultGeneratorID();
    }

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
    
    nextInventoryId = root->get("next-inventory-id", 2);
}

std::unique_ptr<dynamic::Map> World::serialize() const {
    auto root = std::make_unique<dynamic::Map>();

    auto& versionobj = root->putMap("version");
    versionobj.put("major", ENGINE_VERSION_MAJOR);
    versionobj.put("minor", ENGINE_VERSION_MINOR);

    root->put("name", name);
    root->put("generator", generator);
    root->put("seed", seed);
    
    auto& timeobj = root->putMap("time");
    timeobj.put("day-time", daytime);
    timeobj.put("day-time-speed", daytimeSpeed);
    timeobj.put("total-time", totalTime);

    root->put("next-inventory-id", nextInventoryId);
    return root;
}
