#include "LevelScreen.hpp"

#include "../hud.hpp"
#include "../LevelFrontend.hpp"
#include "../../debug/Logger.hpp"
#include "../../audio/audio.hpp"
#include "../../coders/imageio.hpp"
#include "../../graphics/core/PostProcessing.hpp"
#include "../../graphics/core/DrawContext.hpp"
#include "../../graphics/core/Viewport.hpp"
#include "../../graphics/core/ImageData.hpp"
#include "../../graphics/ui/GUI.hpp"
#include "../../graphics/ui/elements/Menu.hpp"
#include "../../graphics/render/WorldRenderer.hpp"
#include "../../logic/LevelController.hpp"
#include "../../logic/scripting/scripting_hud.hpp"
#include "../../physics/Hitbox.hpp"
#include "../../voxels/Chunks.hpp"
#include "../../world/Level.hpp"
#include "../../world/World.hpp"
#include "../../window/Camera.hpp"
#include "../../window/Events.hpp"
#include "../../engine.hpp"

static debug::Logger logger("level-screen");

LevelScreen::LevelScreen(Engine* engine, Level* level)
 : Screen(engine), postProcessing(std::make_unique<PostProcessing>()) 
{
    auto& settings = engine->getSettings();
    auto assets = engine->getAssets();
    auto menu = engine->getGUI()->getMenu();
    menu->reset();

    controller = std::make_unique<LevelController>(settings, level);
    frontend = std::make_unique<LevelFrontend>(controller.get(), assets);

    worldRenderer = std::make_unique<WorldRenderer>(engine, frontend.get(), controller->getPlayer());
    hud = std::make_unique<Hud>(engine, frontend.get(), controller->getPlayer());
    
    keepAlive(settings.graphics.backlight.observe([=](bool) {
        controller->getLevel()->chunks->saveAndClear();
    }));
    keepAlive(settings.camera.fov.observe([=](double value) {
        controller->getPlayer()->camera->setFov(glm::radians(value));
    }));

    animator = std::make_unique<TextureAnimator>();
    animator->addAnimations(assets->getAnimations());

    initializeContent();
}

void LevelScreen::initializeContent() {
    auto content = controller->getLevel()->content;
    for (auto& entry : content->getPacks()) {
        auto pack = entry.second.get();
        const ContentPack& info = pack->getInfo();
        fs::path scriptFile = info.folder/fs::path("scripts/hud.lua");
        if (fs::is_regular_file(scriptFile)) {
            scripting::load_hud_script(pack->getEnvironment(), info.id, scriptFile);
        }
    }
    scripting::on_frontend_init(hud.get());
}

LevelScreen::~LevelScreen() {
    saveWorldPreview();
    scripting::on_frontend_close();
    controller->onWorldQuit();
    engine->getPaths()->setWorldFolder(fs::path());
}

void LevelScreen::saveWorldPreview() {
    try {
        logger.info() << "saving world preview";
        auto paths = engine->getPaths();
        auto player = controller->getPlayer();
        auto& settings = engine->getSettings();
        int previewSize = settings.ui.worldPreviewSize.get();

        // camera special copy for world preview
        Camera camera = *player->camera;
        camera.setFov(glm::radians(70.0f));
        Viewport viewport(previewSize * 1.5, previewSize);
        DrawContext ctx(nullptr, viewport, batch.get());
        
        worldRenderer->draw(ctx, &camera, false, postProcessing.get());
        auto image = postProcessing->toImage();
        image->flipY();
        imageio::write(paths->resolve("world:preview.png").u8string(), image.get());
    } catch (const std::exception& err) {
        logger.error() << err.what();
    }
}

void LevelScreen::updateHotkeys() {
    auto& settings = engine->getSettings();
    if (Events::jpressed(keycode::O)) {
        settings.graphics.frustumCulling.toggle();
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

    bool paused = hud->isPause();
    audio::get_channel("regular")->setPaused(paused);
    audio::get_channel("ambient")->setPaused(paused);
    audio::set_listener(
        camera->position-camera->dir, 
        player->hitbox->velocity,
        camera->dir, 
        glm::vec3(0, 1, 0)
    );

    if (!hud->isPause()) {
        controller->getLevel()->getWorld()->updateTimers(delta);
        animator->update(delta);
    }
    controller->update(delta, !inputLocked, hud->isPause());
    hud->update(hudVisible);
}

void LevelScreen::draw(float) {
    auto camera = controller->getPlayer()->currentCamera;

    Viewport viewport(Window::width, Window::height);
    DrawContext ctx(nullptr, viewport, batch.get());

    worldRenderer->draw(ctx, camera.get(), hudVisible, postProcessing.get());

    if (hudVisible) {
        hud->draw(ctx);
    }
}

void LevelScreen::onEngineShutdown() {
    controller->saveWorld();
}

LevelController* LevelScreen::getLevelController() const {
    return controller.get();
}

