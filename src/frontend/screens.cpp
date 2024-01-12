#include "screens.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <glm/glm.hpp>
#include <filesystem>
#include <stdexcept>

#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../graphics/Shader.h"
#include "../graphics/Batch2D.h"
#include "../graphics/GfxContext.h"
#include "../assets/Assets.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../objects/Player.h"
#include "../logic/ChunksController.h"
#include "../logic/LevelController.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../engine.h"
#include "../util/stringutil.h"
#include "../core_defs.h"
#include "WorldRenderer.h"
#include "hud.h"
#include "ContentGfxCache.h"
#include "gui/GUI.h"
#include "gui/panels.h"
#include "menu.h"

#include "../content/Content.h"
#include "../voxels/Block.h"

using std::string;
using std::wstring;
using glm::vec3;
using glm::vec4;
using std::shared_ptr;

MenuScreen::MenuScreen(Engine* engine_) : Screen(engine_) {
    auto menu = engine->getGUI()->getMenu();
    menus::refresh_menus(engine, menu);
    menu->reset();
    menu->set("main");

    batch = new Batch2D(1024);
    uicamera = new Camera(vec3(), Window::height);
	uicamera->perspective = false;
	uicamera->flipped = true;
}

MenuScreen::~MenuScreen() {
    delete batch;
    delete uicamera;
}

void MenuScreen::update(float delta) {
}

void MenuScreen::draw(float delta) {
    Window::clear();
    Window::setBgColor(vec3(0.2f));

    uicamera->setFov(Window::height);
	Shader* uishader = engine->getAssets()->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjView());

    uint width = Window::width;
    uint height = Window::height;

    batch->begin();
    batch->texture(engine->getAssets()->getTexture("menubg"));
    batch->rect(0, 0, 
                width, height, 0, 0, 0, 
                UVRegion(0, 0, width/64, height/64), 
                false, false, vec4(1.0f));
    batch->render();
}

static bool backlight;

LevelScreen::LevelScreen(Engine* engine, Level* level) 
    : Screen(engine), 
      level(level) {
    auto& settings = engine->getSettings();
    controller = new LevelController(settings, level);
    cache = new ContentGfxCache(level->content, engine->getAssets());
    worldRenderer = new WorldRenderer(engine, level, cache);
    hud = new HudRenderer(engine, level, cache);
    backlight = settings.graphics.backlight;
}

LevelScreen::~LevelScreen() {
    delete controller;
    delete hud;
    delete worldRenderer;
    delete cache;

	std::cout << "-- writing world" << std::endl;
    World* world = level->world;
	world->write(level);

    delete level;
	delete world;
}

void LevelScreen::updateHotkeys() {
    auto& settings = engine->getSettings();
    if (Events::jpressed(keycode::O)) {
        settings.graphics.frustumCulling = !settings.graphics.frustumCulling;
    }
    if (Events::jpressed(keycode::F1)) {
        hudVisible = !hudVisible;
    }
    if (Events::jpressed(keycode::F3)) {
        level->player->debug = !level->player->debug;
    }
    if (Events::jpressed(keycode::F5)) {
        level->chunks->saveAndClear();
    }
}

void LevelScreen::update(float delta) {
    gui::GUI* gui = engine->getGUI();
    
    bool inputLocked = hud->isPause() || 
                       hud->isInventoryOpen() || 
                       gui->isFocusCaught();
    if (!gui->isFocusCaught()) {
        updateHotkeys();
    }

    // TODO: subscribe for setting change
    EngineSettings& settings = engine->getSettings();
    level->player->camera->setFov(glm::radians(settings.camera.fov));
    if (settings.graphics.backlight != backlight) {
        level->chunks->saveAndClear();
        backlight = settings.graphics.backlight;
    }

    if (!hud->isPause()) {
        level->world->updateTimers(delta);
    }
    controller->update(delta, !inputLocked, hud->isPause());
    if (hudVisible)
        hud->update();
}

void LevelScreen::draw(float delta) {
    Camera* camera = level->player->currentViewCamera;

    Viewport viewport(Window::width, Window::height);
    GfxContext ctx(nullptr, viewport, nullptr);

    worldRenderer->draw(ctx, camera);

    if (hudVisible) {
        hud->draw(ctx);
        if (level->player->debug) {
            hud->drawDebug(1 / delta);
        }
    }
}
