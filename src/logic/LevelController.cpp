#include "LevelController.h"
#include "../world/Level.h"

#include "PlayerController.h"
#include "BlocksController.h"
#include "ChunksController.h"

#include "scripting/scripting.h"
#include "../interfaces/Object.h"

LevelController::LevelController(EngineSettings& settings, Level* level) 
    : settings(settings), level(level) {
    blocks = level->spawnObject<BlocksController>(level, settings.chunks.padding);
    chunks = level->spawnObject<ChunksController>(level, settings.chunks.padding);
    player = level->spawnObject<PlayerController>(level, settings, blocks.get());
    shouldUpdate = false;
    scripting::on_world_load(level, blocks.get());
}

LevelController::~LevelController() {
}

void LevelController::updateLevel(float delta, bool input, bool pause) {
    player->updateInput(delta, input, pause);
    level->update();
    chunks->updateChunks(settings.chunks.loadSpeed);

    // erease null pointers
    level->objects.remove_if([](auto obj) { return obj == nullptr; });
    
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

void LevelController::onWorldSave() {
    scripting::on_world_save();
}

void LevelController::onWorldQuit() {
    scripting::on_world_quit();
}
