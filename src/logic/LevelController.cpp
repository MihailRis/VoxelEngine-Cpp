#include "LevelController.h"
#include "../world/Level.h"

#include "PlayerController.h"
#include "BlocksController.h"
#include "ChunksController.h"

#include "scripting/scripting.h"

LevelController::LevelController(EngineSettings& settings, Level* level) 
    : settings(settings), level(level) {
    blocks = new BlocksController(level, settings.chunks.padding);
    chunks = new ChunksController(level, settings.chunks.padding);
    player = new PlayerController(level, settings, blocks);

    scripting::on_world_load(level);
}

LevelController::~LevelController() {
    scripting::on_world_quit();
    delete player;
    delete chunks;
    delete blocks;
}

void LevelController::update(float delta, bool input, bool pause) {
    player->update(delta, input, pause);
    level->update();
    chunks->update(settings.chunks.loadSpeed);
    blocks->update(delta);
}
