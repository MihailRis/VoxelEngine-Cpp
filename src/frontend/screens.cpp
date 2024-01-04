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
#include "../graphics-common/IShader.h"

#ifdef USE_VULKAN
#include "../graphics-vk/Batch2D.h"
#include "../graphics-vk/uniforms/ProjectionViewUniform.h"
#else
#include "../graphics/Batch2D.h"
#endif

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
#include "../graphics-vk/VulkanContext.h"
#include "../graphics-vk/WorldRenderer.h"

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

    batch = new vulkan::Batch2D(1024);
#ifdef USE_VULKAN
    constexpr vec3 camPos = vec3(0, 0, -1);
#else
    constexpr vec3 camPos = vec3();
#endif
    uicamera = new Camera(camPos, static_cast<float>(Window::height));
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
#ifdef USE_VULKAN
    vulkan::VulkanContext::get().beginScreenDraw(0.0f, 0.0f, 0.0f);
#endif

    uicamera->setFov(Window::height);
	IShader* uishader = engine->getAssets()->getShader("ui");
	uishader->use();
#ifdef USE_VULKAN
    const ProjectionViewUniform projectionViewUniform = { uicamera->getProjView() };
    uishader->uniform(projectionViewUniform);
#else
	uishader->uniformMatrix("u_projview", uicamera->getProjView());
#endif

    uint width = Window::width;
    uint height = Window::height;

    batch->begin();
    batch->texture(engine->getAssets()->getTexture("menubg"));
    batch->rect(0, 0, 
                width, height, 0, 0, 0,
                UVRegion(0, 0, width/64, height/64),
                false, false, vec4(1.0f));
    batch->render();
    batch->end();
#ifdef USE_VULKAN
    vulkan::VulkanContext::get().endScreenDraw();
#endif
}

static bool backlight;

LevelScreen::LevelScreen(Engine* engine, Level* level)
    : Screen(engine), 
      level(level) {
    auto& settings = engine->getSettings();
    controller = new LevelController(settings, level);
    cache = new ContentGfxCache(level->content, engine->getAssets());
    worldRenderer = new vulkan::WorldRenderer(engine, level, cache);
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
#ifdef USE_VULKAN
    vulkan::VulkanContext::get().beginScreenDraw(0.0f, 0.0f, 0.0f, VK_ATTACHMENT_LOAD_OP_LOAD);
#endif

    worldRenderer->draw(ctx, camera);

    if (hudVisible) {
        hud->draw(ctx);
        if (level->player->debug) {
            hud->drawDebug(1 / delta);
        }
    }
#ifdef USE_VULKAN
    vulkan::VulkanContext::get().endScreenDraw();
#endif
}
