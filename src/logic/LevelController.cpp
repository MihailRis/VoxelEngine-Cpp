#include "LevelController.h"
#include "../world/Level.h"

#include "PlayerController.h"
#include "ChunksController.h"

LevelController::LevelController(EngineSettings& settings, Level* level) 
    : settings(settings), level(level) {
    chunks = new ChunksController(
                level, 
                level->chunks, 
                level->lighting, 
                settings.chunks.padding);
    player = new PlayerController(level, settings);
}

LevelController::~LevelController() {
    delete player;
    delete chunks;
}

void LevelController::update(
        float delta, 
        bool input, 
        bool pause) {
    player->update(delta, input, pause);
    level->update();
    chunks->update(settings.chunks.loadSpeed);
}