#include "Level.hpp"

#include "content/Content.hpp"
#include "data/dv_util.hpp"
#include "items/Inventories.hpp"
#include "items/Inventory.hpp"
#include "lighting/Lighting.hpp"
#include "objects/Entities.hpp"
#include "objects/Player.hpp"
#include "physics/Hitbox.hpp"
#include "physics/PhysicsSolver.hpp"
#include "settings.hpp"
#include "voxels/Chunk.hpp"
#include "voxels/Chunks.hpp"
#include "voxels/ChunksStorage.hpp"
#include "window/Camera.hpp"
#include "LevelEvents.hpp"
#include "World.hpp"

Level::Level(
    std::unique_ptr<World> worldPtr,
    const Content* content,
    EngineSettings& settings
)
    : world(std::move(worldPtr)),
      content(content),
      chunksStorage(std::make_unique<ChunksStorage>(this)),
      physics(std::make_unique<PhysicsSolver>(glm::vec3(0, -22.6f, 0))),
      events(std::make_unique<LevelEvents>()),
      entities(std::make_unique<Entities>(this)),
      settings(settings) {
    auto& worldInfo = world->getInfo();
    auto& cameraIndices = content->getIndices(ResourceType::CAMERA);
    for (size_t i = 0; i < cameraIndices.size(); i++) {
        auto camera = std::make_shared<Camera>();
        auto map = cameraIndices.getSavedData(i);
        if (map != nullptr) {
            dv::get_vec(map, "pos", camera->position);
            dv::get_mat(map, "rot", camera->rotation);
            map.at("perspective").get(camera->perspective);
            map.at("flipped").get(camera->flipped);
            map.at("zoom").get(camera->zoom);
            float fov = camera->getFov();
            map.at("fov").get(fov);
            camera->setFov(fov);
        }
        camera->updateVectors();
        cameras.push_back(std::move(camera));
    }

    if (worldInfo.nextEntityId) {
        entities->setNextID(worldInfo.nextEntityId);
    }
    auto inv = std::make_shared<Inventory>(
        world->getNextInventoryId(), DEF_PLAYER_INVENTORY_SIZE
    );
    auto player = spawnObject<Player>(
        this, glm::vec3(0, DEF_PLAYER_Y, 0), DEF_PLAYER_SPEED, inv, 0
    );

    uint matrixSize =
        (settings.chunks.loadDistance.get() + settings.chunks.padding.get()) *
        2;
    chunks = std::make_unique<Chunks>(
        matrixSize, matrixSize, 0, 0, world->wfile.get(), this
    );
    lighting = std::make_unique<Lighting>(content, chunks.get());

    events->listen(EVT_CHUNK_HIDDEN, [this](lvl_event_type, Chunk* chunk) {
        this->chunksStorage->remove(chunk->x, chunk->z);
    });

    inventories = std::make_unique<Inventories>(*this);
    inventories->store(player->getInventory());
}

Level::~Level() {
    for (auto obj : objects) {
        obj.reset();
    }
}

void Level::loadMatrix(int32_t x, int32_t z, uint32_t radius) {
    chunks->setCenter(x, z);
    uint32_t diameter = std::min(
        radius * 2LL,
        (settings.chunks.loadDistance.get() + settings.chunks.padding.get()) *
            2LL
    );
    if (chunks->getWidth() != diameter) {
        chunks->resize(diameter, diameter);
    }
}

World* Level::getWorld() {
    return world.get();
}

const World* Level::getWorld() const {
    return world.get();
}

void Level::onSave() {
    auto& cameraIndices = content->getIndices(ResourceType::CAMERA);
    for (size_t i = 0; i < cameraIndices.size(); i++) {
        auto& camera = *cameras.at(i);
        auto map = dv::object();
        map["pos"] = dv::to_value(camera.position);
        map["rot"] = dv::to_value(camera.rotation);
        map["perspective"] = camera.perspective;
        map["flipped"] = camera.flipped;
        map["zoom"] = camera.zoom;
        map["fov"] = camera.getFov();
        cameraIndices.saveData(i, std::move(map));
    }
}

std::shared_ptr<Camera> Level::getCamera(const std::string& name) {
    size_t index = content->getIndices(ResourceType::CAMERA).indexOf(name);
    if (index == ResourceIndices::MISSING) {
        return nullptr;
    }
    return cameras.at(index);
}
