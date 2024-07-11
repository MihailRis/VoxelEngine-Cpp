#include "Level.hpp"
#include "World.hpp"
#include "LevelEvents.hpp"
#include "../settings.hpp"
#include "../content/Content.hpp"
#include "../lighting/Lighting.hpp"
#include "../voxels/Chunk.hpp"
#include "../voxels/Chunks.hpp"
#include "../voxels/ChunksStorage.hpp"
#include "../physics/Hitbox.hpp"
#include "../physics/PhysicsSolver.hpp"
#include "../objects/Player.hpp"
#include "../objects/Entities.hpp"
#include "../items/Inventory.hpp"
#include "../items/Inventories.hpp"
#include "../window/Camera.hpp"
#include "../data/dynamic_util.hpp"

Level::Level(
    std::unique_ptr<World> worldPtr,
    const Content* content,
    EngineSettings& settings
) : world(std::move(worldPtr)),
	content(content),
	chunksStorage(std::make_unique<ChunksStorage>(this)),
	physics(std::make_unique<PhysicsSolver>(glm::vec3(0, -22.6f, 0))),
    events(std::make_unique<LevelEvents>()),
    entities(std::make_unique<Entities>(this)),
	settings(settings)
{
    if (world->nextEntityId) {
        entities->setNextID(world->nextEntityId);
    }
	auto inv = std::make_shared<Inventory>(
        world->getNextInventoryId(), DEF_PLAYER_INVENTORY_SIZE
    );
	auto player = spawnObject<Player>(
        this, glm::vec3(0, DEF_PLAYER_Y, 0), DEF_PLAYER_SPEED, inv, 0
    );

    uint matrixSize = (
        settings.chunks.loadDistance.get() + 
        settings.chunks.padding.get()
    ) * 2;
    chunks = std::make_unique<Chunks>(
		matrixSize, matrixSize, 0, 0, world->wfile.get(), this
	);
	lighting = std::make_unique<Lighting>(content, chunks.get());

	events->listen(EVT_CHUNK_HIDDEN, [this](lvl_event_type, Chunk* chunk) {
		this->chunksStorage->remove(chunk->x, chunk->z);
	});

	inventories = std::make_unique<Inventories>(*this);
	inventories->store(player->getInventory());

    auto& cameraIndices = content->getIndices(ResourceType::CAMERA);
    for (size_t i = 0; i < cameraIndices.size(); i++) {
        auto camera = std::make_shared<Camera>();
        if (auto map = cameraIndices.getSavedData(i)) {
            dynamic::get_vec(map, "pos", camera->position);
            dynamic::get_vec(map, "front", camera->front);
            dynamic::get_vec(map, "up", camera->up);
            map->flag("perspective", camera->perspective);
            map->flag("flipped", camera->flipped);
            map->num("zoom", camera->zoom);
            float fov = camera->getFov();
            map->num("fov", fov);
            camera->setFov(fov);
        }
        cameras.push_back(std::move(camera));
    }
}

Level::~Level(){
	for(auto obj : objects) {
		obj.reset();
	}
}

void Level::loadMatrix(int32_t x, int32_t z, uint32_t radius) {
	chunks->setCenter(x, z);
    uint32_t diameter = std::min(
        radius*2LL,
        (settings.chunks.loadDistance.get() + settings.chunks.padding.get()) * 2LL
    );
	if (chunks->w != diameter) {
		chunks->resize(diameter, diameter);
	}
}

World* Level::getWorld() {
    return world.get();
}

void Level::onSave() {
    auto& cameraIndices = content->getIndices(ResourceType::CAMERA);
    for (size_t i = 0; i < cameraIndices.size(); i++) {
        auto& camera = *cameras.at(i);
        auto map = dynamic::create_map();
        map->put("pos", dynamic::to_value(camera.position));
        map->put("front", dynamic::to_value(camera.front));
        map->put("up", dynamic::to_value(camera.up));
        map->put("perspective", camera.perspective);
        map->put("flipped", camera.flipped);
        map->put("zoom", camera.zoom);
        map->put("fov", camera.getFov());
        cameraIndices.saveData(i, std::move(map));
    }
}
