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
        bool pause, 
        bool interactions) {
    if (!pause) {
		if (input) {
			player->updateKeyboard();
		} else {
			player->resetKeyboard();
		}
        player->updateCamera(delta, input);
		player->updateControls(delta);

	}
	player->refreshCamera();
	if (interactions) {
		player->updateInteraction();
	} else {
		player->selectedBlockId = -1;
	}
    level->update();
    chunks->update(settings.chunks.loadSpeed);
}