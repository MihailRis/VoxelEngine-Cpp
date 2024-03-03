#include "screens.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <glm/glm.hpp>
#include <filesystem>
#include <stdexcept>

#include "../audio/audio.h"
#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../graphics/Shader.h"
#include "../graphics/Batch2D.h"
#include "../graphics/GfxContext.h"
#include "../graphics/TextureAnimation.h"
#include "../assets/Assets.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../logic/ChunksController.h"
#include "../logic/LevelController.h"
#include "../logic/scripting/scripting.h"
#include "../logic/scripting/scripting_frontend.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../engine.h"
#include "../util/stringutil.h"
#include "../core_defs.h"
#include "WorldRenderer.h"
#include "hud.h"
#include "ContentGfxCache.h"
#include "LevelFrontend.h"
#include "gui/GUI.h"
#include "gui/containers.h"
#include "menu/menu.h"

#include "../content/Content.h"
#include "../voxels/Block.h"

Screen::Screen(Engine* engine) : engine(engine), batch(new Batch2D(1024)) {
}

Screen::~Screen() {
}

MenuScreen::MenuScreen(Engine* engine_) : Screen(engine_) {
    auto menu = engine->getGUI()->getMenu();
    menus::refresh_menus(engine);
    menu->reset();
    menu->setPage("main");

    uicamera.reset(new Camera(glm::vec3(), Window::height));
    uicamera->perspective = false;
    uicamera->flipped = true;
}

MenuScreen::~MenuScreen() {
}

void MenuScreen::update(float delta) {
}

void MenuScreen::draw(float delta) {
    Window::clear();
    Window::setBgColor(glm::vec3(0.2f));

    uicamera->setFov(Window::height);
    Shader* uishader = engine->getAssets()->getShader("ui");
    uishader->use();
    uishader->uniformMatrix("u_projview", uicamera->getProjView());

    uint width = Window::width;
    uint height = Window::height;

    batch->begin();
    batch->texture(engine->getAssets()->getTexture("gui/menubg"));
    batch->rect(
        0, 0, 
        width, height, 0, 0, 0, 
        UVRegion(0, 0, width/64, height/64), 
        false, false, glm::vec4(1.0f)
    );
    batch->flush();
}

static bool backlight;

LevelScreen::LevelScreen(Engine* engine, Level* level) : Screen(engine) {
    auto& settings = engine->getSettings();
    auto assets = engine->getAssets();

    controller = std::make_unique<LevelController>(settings, level);
    frontend = std::make_unique<LevelFrontend>(level, assets);
    worldRenderer = std::make_unique<WorldRenderer>(engine, frontend.get(), controller->getPlayer());
    hud = std::make_unique<Hud>(engine, frontend.get(), controller->getPlayer());

    backlight = settings.graphics.backlight;

    animator = std::make_unique<TextureAnimator>();
    animator->addAnimations(assets->getAnimations());

    auto content = level->content;
    for (auto& entry : content->getPacks()) {
        auto pack = entry.second.get();
        const ContentPack& info = pack->getInfo();
        fs::path scriptFile = info.folder/fs::path("scripts/hud.lua");
        if (fs::is_regular_file(scriptFile)) {
            scripting::load_hud_script(pack->getEnvironment()->getId(), info.id, scriptFile);
        }
    }
    scripting::on_frontend_init(hud.get());
}

LevelScreen::~LevelScreen() {
    std::cout << "-- writing world" << std::endl;
    scripting::on_frontend_close();
    controller->onWorldSave();
    auto world = controller->getLevel()->getWorld();
    world->write(controller->getLevel());
    controller->onWorldQuit();
    engine->getPaths()->setWorldFolder(fs::path());
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
        controller->getPlayer()->debug = !controller->getPlayer()->debug;
    }
    if (Events::jpressed(keycode::F5)) {
        controller->getLevel()->chunks->saveAndClear();
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

    auto player = controller->getPlayer();
    auto camera = player->camera;
    audio::set_listener(
        camera->position, 
        player->hitbox->velocity,
        camera->position+camera->dir, 
        camera->up
    );

    // TODO: subscribe for setting change
    EngineSettings& settings = engine->getSettings();
    controller->getPlayer()->camera->setFov(glm::radians(settings.camera.fov));
    if (settings.graphics.backlight != backlight) {
        controller->getLevel()->chunks->saveAndClear();
        backlight = settings.graphics.backlight;
    }

    if (!hud->isPause()) {
        controller->getLevel()->world->updateTimers(delta);
        animator->update(delta);
    }
    controller->update(delta, !inputLocked, hud->isPause());
    hud->update(hudVisible);
}

void LevelScreen::draw(float delta) {
    auto camera = controller->getPlayer()->currentCamera;

    Viewport viewport(Window::width, Window::height);
    GfxContext ctx(nullptr, viewport, batch.get());

    worldRenderer->draw(ctx, camera.get(), hudVisible);

    if (hudVisible) {
        hud->draw(ctx);
        if (controller->getPlayer()->debug) {
            hud->drawDebug(1 / delta);
        }
    }
}
