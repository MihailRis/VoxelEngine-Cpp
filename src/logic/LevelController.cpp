#include "LevelController.hpp"

#include <algorithm>

#include "debug/Logger.hpp"
#include "engine.hpp"
#include "files/WorldFiles.hpp"
#include "maths/voxmaths.hpp"
#include "objects/Entities.hpp"
#include "objects/Players.hpp"
#include "objects/Player.hpp"
#include "physics/Hitbox.hpp"
#include "scripting/scripting.hpp"
#include "settings.hpp"
#include "world/Level.hpp"
#include "world/World.hpp"

static debug::Logger logger("level-control");

LevelController::LevelController(Engine* engine, std::unique_ptr<Level> levelPtr)
    : settings(engine->getSettings()),
      level(std::move(levelPtr)),
      chunks(std::make_unique<ChunksController>(
          *level, settings.chunks.padding.get()
      )) {
    blocks = std::make_unique<BlocksController>(
        *level, *chunks->lighting, settings.chunks.padding.get()
    );
    scripting::on_world_load(this);
}

void LevelController::update(float delta, bool pause) {
    for (const auto& [uid, player] : *level->players) {
        glm::vec3 position = player->getPosition();
        level->loadMatrix(
            position.x,
            position.z,
            settings.chunks.loadDistance.get() + settings.chunks.padding.get() * 2
        );
        chunks->update(
            settings.chunks.loadSpeed.get(), settings.chunks.loadDistance.get(),
            floordiv(position.x, CHUNK_W), floordiv(position.z, CHUNK_D)
        );
    }
    if (!pause) {
        // update all objects that needed
        blocks->update(delta);
        level->entities->updatePhysics(delta);
        level->entities->update(delta);
    }
    level->entities->clean();
}

void LevelController::saveWorld() {
    auto world = level->getWorld();
    logger.info() << "writing world '" << world->getName() << "'";
    world->wfile->createDirectories();
    scripting::on_world_save();
    level->onSave();
    level->getWorld()->write(level.get());
}

void LevelController::onWorldQuit() {
    scripting::on_world_quit();
}

Level* LevelController::getLevel() {
    return level.get();
}

BlocksController* LevelController::getBlocksController() {
    return blocks.get();
}

ChunksController* LevelController::getChunksController() {
    return chunks.get();
}
