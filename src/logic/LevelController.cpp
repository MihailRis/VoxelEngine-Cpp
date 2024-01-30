#include "LevelController.h"
#include "../world/Level.h"

#include "PlayerController.h"
#include "BlocksController.h"
#include "ChunksController.h"

#include "scripting/scripting.h"
#include "../objects/Object.h"

LevelController::LevelController(EngineSettings& settings, Level* level) 
    : settings(settings), level(level) {
    blocks = std::make_unique<BlocksController>(level, settings.chunks.padding);
    chunks = std::make_unique<ChunksController>(level, settings.chunks.padding);
    player = std::make_unique<PlayerController>(level, settings, blocks.get());

    scripting::on_world_load(level, blocks.get());
}

LevelController::~LevelController() {
}

void LevelController::update(float delta, bool input, bool pause) {
    player->update(delta, input, pause);
    level->update();
    chunks->update(settings.chunks.loadSpeed);
    blocks->update(delta);

    // erease null pointers
    level->objects.remove_if([](Object* obj) { return obj == nullptr; });

    // update all objects that needed
	for(Object *obj : level->objects)
	{
		if(obj) {
            if(obj->shouldUpdate) {
                obj->update(delta);
            }
		}
	}
}

void LevelController::onWorldSave() {
    scripting::on_world_save();
}

void LevelController::onWorldQuit() {
    scripting::on_world_quit();
}
