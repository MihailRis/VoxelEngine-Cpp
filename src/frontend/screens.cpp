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
#include "../voxels/ChunksController.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "world_render.h"
#include "hud.h"
#include "ContentGfxCache.h"
#include "gui/GUI.h"
#include "gui/panels.h"
#include "../engine.h"
#include "../util/stringutil.h"
#include "../core_defs.h"

#include "menu.h"

using std::string;
using std::wstring;
using glm::vec3;
using glm::vec4;
using std::shared_ptr;

MenuScreen::MenuScreen(Engine* engine_) : Screen(engine_) {
    auto menu = engine->getGUI()->getMenu();

    // Create pages if not created yet
    if (!menu->has("new-world"))
        menu->add("new-world", create_new_world_panel(engine, menu));
    if (!menu->has("settings"))
        menu->add("settings", create_settings_panel(engine, menu));
    if (!menu->has("controls"))
        menu->add("controls", create_controls_panel(engine, menu));
    if (!menu->has("pause"))
        menu->add("pause", create_pause_panel(engine, menu));

    menu->add("main", create_main_menu_panel(engine, menu));
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
    Window::setBgColor(vec3(0.2f, 0.2f, 0.2f));

    uicamera->fov = Window::height;
	Shader* uishader = engine->getAssets()->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjView());

    batch->begin();
    batch->texture(engine->getAssets()->getTexture("menubg"));
    batch->rect(0, 0, 
                Window::width, Window::height, 0, 0, 0, 
                UVRegion(0, 0, Window::width/64, Window::height/64), 
                false, false, vec4(1.0f));
    batch->render();
}

static bool backlight;
LevelScreen::LevelScreen(Engine* engine, Level* level) 
    : Screen(engine), 
      level(level) {
    cache = new ContentGfxCache(level->content, engine->getAssets());
    worldRenderer = new WorldRenderer(engine, level, cache);
    hud = new HudRenderer(engine, level, cache);
    backlight = engine->getSettings().graphics.backlight;
}

LevelScreen::~LevelScreen() {
    delete hud;
    delete worldRenderer;
    delete cache;

	std::cout << "-- writing world" << std::endl;
    World* world = level->world;
	world->write(level, !engine->getSettings().debug.generatorTestMode);

    delete level;
	delete world;
}

void LevelScreen::updateHotkeys() {
    if (Events::jpressed(keycode::O)) {
        occlusion = !occlusion;
    }
    if (Events::jpressed(keycode::F3)) {
        level->player->debug = !level->player->debug;
    }
    if (Events::jpressed(keycode::F5)) {
        level->chunks->clear();
    }
}

void LevelScreen::update(float delta) {
    gui::GUI* gui = engine->getGUI();
    EngineSettings& settings = engine->getSettings();

    bool inputLocked = hud->isPause() || 
                       hud->isInventoryOpen() || 
                       gui->isFocusCaught();
    if (!gui->isFocusCaught()) {
        updateHotkeys();
    }
    if (settings.graphics.backlight != backlight) {
        level->chunks->clear();
        backlight = settings.graphics.backlight;
    }
    level->updatePlayer(delta, !inputLocked, hud->isPause(), !inputLocked);
    level->update();
    
    level->chunksController->update(settings.chunks.loadSpeed);

    hud->update();
}

void LevelScreen::draw(float delta) {
    Camera* camera = level->player->camera;

    Viewport viewport(Window::width, Window::height);
    GfxContext ctx(nullptr, viewport, nullptr);

    worldRenderer->draw(ctx, camera, occlusion);
    hud->draw(ctx);
    if (level->player->debug) {
        hud->drawDebug( 1 / delta, occlusion);
    }
}
