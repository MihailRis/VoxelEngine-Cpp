#include "LevelController.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../voxels/ChunksStorage.h"

#include "scripting/scripting.h"
#include "../interfaces/Object.h"

#include <iostream>

LevelController::LevelController(EngineSettings& settings, Level* level) 
    : settings(settings), level(level),
    blocks(std::make_unique<BlocksController>(level)),
    player(std::make_unique<PlayerController>(level, settings, blocks.get())) {
    scripting::on_world_load(level, blocks.get());
}

void LevelController::update(float delta, bool input, bool pause) {
    player->getPlayer()->radius = settings.chunks.loadDistance;
    player->update(delta, input, pause);
    for(auto obj : level->objects) {
        if (auto player = std::dynamic_pointer_cast<Player>(obj)) {
            player->loadChunks();
        }
    }
    level->chunksStorage->unloadUnused();

    // erease null pointers
    level->objects.erase(
        std::remove_if(
            level->objects.begin(), level->objects.end(),
            [](auto obj) { return obj == nullptr; }),
        level->objects.end()
    );
    
    if (!pause) {
        // update all objects that needed
        for(auto obj : level->objects)
        {
            if(obj) {
                if(obj->shouldUpdate) {
                    obj->update(delta);
                }
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

PlayerController* LevelController::getPlayerController() {
    return player.get();
}
