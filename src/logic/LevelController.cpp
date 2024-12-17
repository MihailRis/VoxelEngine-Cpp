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
#include "voxels/Chunks.hpp"
#include "scripting/scripting.hpp"
#include "lighting/Lighting.hpp"
#include "settings.hpp"
#include "world/Level.hpp"
#include "world/World.hpp"

static debug::Logger logger("level-control");

LevelController::LevelController(
    Engine* engine, std::unique_ptr<Level> levelPtr, Player* clientPlayer
)
    : settings(engine->getSettings()),
      level(std::move(levelPtr)),
      chunks(std::make_unique<ChunksController>(
          *level, settings.chunks.padding.get()
      )) {
    
    if (clientPlayer) {
        chunks->lighting = std::make_unique<Lighting>(
            level->content, clientPlayer->chunks.get()
        );
    }
    blocks = std::make_unique<BlocksController>(
        *level,
        chunks ? chunks->lighting.get() : nullptr,
        settings.chunks.padding.get()
    );
    scripting::on_world_load(this);
}

void LevelController::update(float delta, bool pause) {
    for (const auto& [uid, player] : *level->players) {
        glm::vec3 position = player->getPosition();
        player->chunks->configure(
            position.x,
            position.z,
            settings.chunks.loadDistance.get() + settings.chunks.padding.get()
        );
        chunks->update(
            settings.chunks.loadSpeed.get(),
            settings.chunks.loadDistance.get(),
            *player
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
