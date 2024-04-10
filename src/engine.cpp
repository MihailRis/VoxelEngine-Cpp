#include "engine.h"

#define GLEW_STATIC

#include "assets/AssetsLoader.h"
#include "audio/audio.h"
#include "coders/GLSLExtension.h"
#include "coders/json.h"
#include "coders/png.h"
#include "content/ContentLoader.h"
#include "core_defs.h"
#include "files/files.h"
#include "frontend/locale/langs.h"
#include "frontend/menu/menu.h"
#include "frontend/screens.h"
#include "graphics/core/Batch2D.h"
#include "graphics/core/GfxContext.h"
#include "graphics/core/ImageData.h"
#include "graphics/core/Shader.h"
#include "graphics/ui/GUI.h"
#include "logic/scripting/scripting.h"
#include "util/listutil.h"
#include "util/platform.h"
#include "voxels/DefaultWorldGenerator.h"
#include "voxels/FlatWorldGenerator.h"
#include "window/Camera.h"
#include "window/Events.h"
#include "window/input.h"
#include "window/Window.h"
#include "world/WorldGenerators.h"

#include <iostream>
#include <assert.h>
#include <glm/glm.hpp>
#include <unordered_set>
#include <functional>

#include "debug/Logger.h"

static debug::Logger logger("engine");

namespace fs = std::filesystem;

void addWorldGenerators() {
    WorldGenerators::addGenerator<DefaultWorldGenerator>("core:default");
    WorldGenerators::addGenerator<FlatWorldGenerator>("core:flat");
}

inline void create_channel(std::string name, NumberSetting& setting) {
    if (name != "master") {
        audio::create_channel(name);
    }
    setting.observe([=](auto value) {
        audio::get_channel(name)->setVolume(value*value);
    });
}

Engine::Engine(EngineSettings& settings, EnginePaths* paths) 
    : settings(settings), settingsHandler(settings), paths(paths) 
{    
    if (Window::initialize(settings.display)){
        throw initialize_error("could not initialize window");
    }
    audio::initialize(settings.audio.enabled);
    create_channel("master", settings.audio.volumeMaster);
    create_channel("regular", settings.audio.volumeRegular);
    create_channel("music", settings.audio.volumeMusic);
    create_channel("ambient", settings.audio.volumeAmbient);
    create_channel("ui", settings.audio.volumeUI);

    auto resdir = paths->getResources();

    logger.info() << "loading assets";
    std::vector<fs::path> roots {resdir};
    resPaths = std::make_unique<ResPaths>(resdir, roots);
    try {
        loadAssets();
    } catch (std::runtime_error& err) {
        logger.error() << "fatal error occurred while loading assets";

        assets.reset();
        scripting::close();
        audio::close();
        Window::terminate();
        throw initialize_error(err.what());
    }

    gui = std::make_unique<gui::GUI>();
    if (settings.ui.language == "auto") {
        settings.ui.language = langs::locale_by_envlocale(
            platform::detect_locale(),
            paths->getResources()
        );
    }
    if (ENGINE_VERSION_INDEV) {
        menus::create_version_label(this);
    }
    setLanguage(settings.ui.language);
    addWorldGenerators();
    onAssetsLoaded();
    
    scripting::initialize(this);
}

void Engine::onAssetsLoaded() {
    gui->onAssetsLoad(assets.get());
}

void Engine::updateTimers() {
    frame++;
    double currentTime = Window::time();
    delta = currentTime - lastTime;
    lastTime = currentTime;
}

void Engine::updateHotkeys() {
    if (Events::jpressed(keycode::F2)) {
        std::unique_ptr<ImageData> image(Window::takeScreenshot());
        image->flipY();
        fs::path filename = paths->getScreenshotFile("png");
        png::write_image(filename.string(), image.get());
        std::cout << "saved screenshot as " << filename << std::endl;
    }
    if (Events::jpressed(keycode::F11)) {
        Window::toggleFullscreen();
    }
}

void Engine::mainloop() {
    logger.info() << "starting menu screen";
    setScreen(std::make_shared<MenuScreen>(this));

    Batch2D batch(1024);
    lastTime = Window::time();
    
    logger.info() << "engine started";
    while (!Window::isShouldClose()){
        assert(screen != nullptr);
        updateTimers();
        updateHotkeys();
        audio::update(delta);

        gui->act(delta);
        screen->update(delta);

        if (!Window::isIconified()) {
            renderFrame(batch);
        }
        Window::swapInterval(Window::isIconified() ? 1 : settings.display.swapInterval);

        processPostRunnables();

        Window::swapBuffers();
        Events::pollEvents();
    }
}

void Engine::renderFrame(Batch2D& batch) {
    screen->draw(delta);

    Viewport viewport(Window::width, Window::height);
    GfxContext ctx(nullptr, viewport, &batch);
    gui->draw(&ctx, assets.get());
}

void Engine::processPostRunnables() {
    std::lock_guard<std::recursive_mutex> lock(postRunnablesMutex);
    while (!postRunnables.empty()) {
        postRunnables.front()();
        postRunnables.pop();
    }
    scripting::process_post_runnables();
}

Engine::~Engine() {
    logger.info() << "shutting down";
    if (screen) {
        screen->onEngineShutdown();
    }
    screen.reset();
    content.reset();
    assets.reset();
    audio::close();
    scripting::close();
    Window::terminate();
    logger.info() << "engine finished";
}

PacksManager Engine::createPacksManager(const fs::path& worldFolder) {
    PacksManager manager;
    manager.setSources({
        worldFolder/fs::path("content"),
        paths->getUserfiles()/fs::path("content"),
        paths->getResources()/fs::path("content")
    });
    return manager;
}

void Engine::loadAssets() {
    logger.info() << "loading assets";
    Shader::preprocessor->setPaths(resPaths.get());

    auto new_assets = std::make_unique<Assets>();
    AssetsLoader loader(new_assets.get(), resPaths.get());
    AssetsLoader::addDefaults(loader, content.get());
    while (loader.hasNext()) {
        if (!loader.loadNext()) {
            new_assets.reset();
            throw std::runtime_error("could not to load assets");
        }
    }
    if (assets) {
        assets->extend(*new_assets);
    } else {
        assets.reset(new_assets.release());
    }
}

// TODO: refactor this
void Engine::loadContent() {
    auto resdir = paths->getResources();
    ContentBuilder contentBuilder;
    corecontent::setup(&contentBuilder);
    paths->setContentPacks(&contentPacks);

    std::vector<std::string> names;
    for (auto& pack : contentPacks) {
        names.push_back(pack.id);
    }
    PacksManager manager = createPacksManager(paths->getWorldFolder());
    manager.scan();
    names = manager.assembly(names);
    contentPacks = manager.getAll(names);

    std::vector<fs::path> resRoots;
    for (auto& pack : contentPacks) {
        resRoots.push_back(pack.folder);

        ContentLoader loader(&pack);
        loader.load(contentBuilder);
    }
    content.reset(contentBuilder.build());
    resPaths = std::make_unique<ResPaths>(resdir, resRoots);

    langs::setup(resdir, langs::current->getId(), contentPacks);
    loadAssets();
    onAssetsLoaded();
}

void Engine::loadWorldContent(const fs::path& folder) {
    contentPacks.clear();
    auto packNames = ContentPack::worldPacksList(folder);
    PacksManager manager;
    manager.setSources({
        folder/fs::path("content"),
        paths->getUserfiles()/fs::path("content"),
        paths->getResources()/fs::path("content")
    });
    manager.scan();
    contentPacks = manager.getAll(manager.assembly(packNames));
    paths->setWorldFolder(folder);
    loadContent();
}

void Engine::loadAllPacks() {
    PacksManager manager;
    manager.setSources({
        paths->getWorldFolder()/fs::path("content"),
        paths->getUserfiles()/fs::path("content"),
        paths->getResources()/fs::path("content")
    });
    manager.scan();
    auto allnames = manager.getAllNames();
    contentPacks = manager.getAll(manager.assembly(allnames));
}

double Engine::getDelta() const {
    return delta;
}

void Engine::setScreen(std::shared_ptr<Screen> screen) {
    audio::reset_channel(audio::get_channel_index("regular"));
    audio::reset_channel(audio::get_channel_index("ambient"));
    this->screen = screen;
}

void Engine::setLanguage(std::string locale) {
    settings.ui.language = locale;
    langs::setup(paths->getResources(), locale, contentPacks);
    menus::create_menus(this);
}

gui::GUI* Engine::getGUI() {
    return gui.get();
}

EngineSettings& Engine::getSettings() {
    return settings;
}

Assets* Engine::getAssets() {
    return assets.get();
}

const Content* Engine::getContent() const {
    return content.get();
}

std::vector<ContentPack>& Engine::getContentPacks() {
    return contentPacks;
}

EnginePaths* Engine::getPaths() {
    return paths;
}

ResPaths* Engine::getResPaths() {
    return resPaths.get();
}

std::shared_ptr<Screen> Engine::getScreen() {
    return screen;
}

void Engine::postRunnable(runnable callback) {
    std::lock_guard<std::recursive_mutex> lock(postRunnablesMutex);
    postRunnables.push(callback);
}

SettingsHandler& Engine::getSettingsHandler() {
    return settingsHandler;
}
