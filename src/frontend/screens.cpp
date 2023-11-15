#include "screens.h"

#include <iostream>
#include <memory>

#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../assets/Assets.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../objects/Player.h"
#include "../voxels/ChunksController.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "world_render.h"
#include "hud.h"
#include "gui/GUI.h"
#include "../engine.h"

using std::shared_ptr;

LevelScreen::LevelScreen(Engine* engine, Level* level) : Screen(engine), level(level) {
    worldRenderer = new WorldRenderer(level, engine->getAssets());
    hud = new HudRenderer(engine->getGUI(), level, engine->getAssets());
}

LevelScreen::~LevelScreen() {
    delete hud;
    delete worldRenderer;


    World* world = level->world;

	std::cout << "-- saving world" << std::endl;
	world->write(level, !engine->getSettings().debug.generatorTestMode);

	delete world;
    delete level;

}

void LevelScreen::updateHotkeys() {
    if (Events::jpressed(keycode::O)) {
        occlusion = !occlusion;
    }
    if (Events::jpressed(keycode::F3)) {
        level->player->debug = !level->player->debug;
    }
    if (Events::jpressed(keycode::F5)) {
        for (uint i = 0; i < level->chunks->volume; i++) {
            shared_ptr<Chunk> chunk = level->chunks->chunks[i];
            if (chunk != nullptr && chunk->isReady()) {
                chunk->setModified(true);
            }
        }
    }
}

void LevelScreen::update(float delta) {
    gui::GUI* gui = engine->getGUI();
    EngineSettings& settings = engine->getSettings();

    bool inputLocked = hud->isPause() || hud->isInventoryOpen() || gui->isFocusCaught();
    if (!inputLocked) {
        updateHotkeys();
    }
    level->updatePlayer(delta, !inputLocked, hud->isPause(), !inputLocked);
    level->update();
    level->chunksController->update(settings.chunks.loadSpeed);
}

void LevelScreen::draw(float delta) {
    EngineSettings& settings = engine->getSettings();
    Camera* camera = level->player->camera;

    float fovFactor = 18.0f / (float)settings.chunks.loadDistance;
    worldRenderer->draw(camera, occlusion, fovFactor, settings.graphics.fogCurve);
    hud->draw();
    if (level->player->debug) {
        hud->drawDebug( 1 / delta, occlusion);
    }
}