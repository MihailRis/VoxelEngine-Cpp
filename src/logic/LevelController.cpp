#include "LevelController.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../physics/Hitbox.h"

#include "scripting/scripting.h"
#include "../interfaces/Object.h"

#include <iostream>

LevelController::LevelController(EngineSettings& settings, Level* level) 
    : settings(settings), level(level),
    blocks(std::make_unique<BlocksController>(level, settings.chunks.padding)),
    chunks(std::make_unique<ChunksController>(level, settings.chunks.padding)),
    player(std::make_unique<PlayerController>(level, settings, blocks.get())) {

    scripting::on_world_load(this);
}

void LevelController::update(float delta, bool input, bool pause) {
    player->update(delta, input, pause);
	glm::vec3 position = player->getPlayer()->hitbox->position;
    level->loadMatrix(position.x, position.z, settings.chunks.loadDistance + settings.chunks.padding * 2);
    chunks->update(settings.chunks.loadSpeed);

    // erease null pointers
    level->objects.erase(
        std::remove_if(
            level->objects.begin(), level->objects.end(),
            [](auto obj) { return obj == nullptr; }),
        level->objects.end()
    );
    
    if (!pause) {
        // update all objects that needed
        for (auto obj : level->objects) {
            if (obj && obj->shouldUpdate) {
                obj->update(delta);
            }
        }
        blocks->update(delta);
    }
}

void LevelController::saveWorld() {
    std::cout << "-- writing world" << std::endl;
    scripting::on_world_save();
    level->getWorld()->write(level.get());
}

void LevelController::onWorldQuit() {
    scripting::on_world_quit();
}

Level* LevelController::getLevel() {
    return level.get();
}

Player* LevelController::getPlayer() {
    return player->getPlayer();
}

BlocksController* LevelController::getBlocksController() {
    return blocks.get();
}

PlayerController* LevelController::getPlayerController() {
    return player.get();
}
