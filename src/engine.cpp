#include "engine.hpp"

#define GLEW_STATIC

#include "debug/Logger.hpp"
#include "assets/AssetsLoader.hpp"
#include "audio/audio.hpp"
#include "coders/GLSLExtension.hpp"
#include "coders/imageio.hpp"
#include "coders/json.hpp"
#include "coders/toml.hpp"
#include "content/ContentBuilder.hpp"
#include "content/ContentLoader.hpp"
#include "core_defs.hpp"
#include "files/files.hpp"
#include "files/settings_io.hpp"
#include "frontend/locale.hpp"
#include "frontend/menu.hpp"
#include "frontend/screens/Screen.hpp"
#include "frontend/screens/MenuScreen.hpp"
#include "graphics/core/Batch2D.hpp"
#include "graphics/core/DrawContext.hpp"
#include "graphics/core/ImageData.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/ui/GUI.hpp"
#include "logic/EngineController.hpp"
#include "logic/CommandsInterpreter.hpp"
#include "logic/scripting/scripting.hpp"
#include "util/listutil.hpp"
#include "util/platform.hpp"
#include "voxels/DefaultWorldGenerator.hpp"
#include "voxels/FlatWorldGenerator.hpp"
#include "voxels/SkygridWorldGenerator.hpp"
#include "window/Camera.hpp"
#include "window/Events.hpp"
#include "window/input.hpp"
#include "window/Window.hpp"
#include "world/WorldGenerators.hpp"
#include "settings.hpp"

#include <iostream>
#include <assert.h>
#include <glm/glm.hpp>
#include <unordered_set>
#include <functional>
#include <utility>

static debug::Logger logger("engine");

namespace fs = std::filesystem;

void addWorldGenerators() {
    WorldGenerators::addGenerator<DefaultWorldGenerator>("core:default");
    WorldGenerators::addGenerator<FlatWorldGenerator>("core:flat");
    WorldGenerators::addGenerator<SkygridWorldGenerator>("core:skygrid");
}

inline void create_channel(Engine* engine, std::string name, NumberSetting& setting) {
    if (name != "master") {
        audio::create_channel(name);
    }
    engine->keepAlive(setting.observe([=](auto value) {
        audio::get_channel(name)->setVolume(value*value);
    }, true));
}

Engine::Engine(EngineSettings& settings, SettingsHandler& settingsHandler, EnginePaths* paths) 
    : settings(settings), settingsHandler(settingsHandler), paths(paths),
      interpreter(std::make_unique<cmd::CommandsInterpreter>())
{
    paths->prepare();
    loadSettings();

    controller = std::make_unique<EngineController>(this);
    if (Window::initialize(&this->settings.display)){
        throw initialize_error("could not initialize window");
    }
    loadControls();
    audio::initialize(settings.audio.enabled.get());
    create_channel(this, "master", settings.audio.volumeMaster);
    create_channel(this, "regular", settings.audio.volumeRegular);
    create_channel(this, "music", settings.audio.volumeMusic);
    create_channel(this, "ambient", settings.audio.volumeAmbient);
    create_channel(this, "ui", settings.audio.volumeUI);

    gui = std::make_unique<gui::GUI>();
    if (settings.ui.language.get() == "auto") {
        settings.ui.language.set(langs::locale_by_envlocale(
            platform::detect_locale(),
            paths->getResources()
        ));
    }
    if (ENGINE_DEBUG_BUILD) {
        menus::create_version_label(this);
    }
    keepAlive(settings.ui.language.observe([=](auto lang) {
        setLanguage(lang);
    }, true));
    addWorldGenerators();
    
    scripting::initialize(this);

    auto resdir = paths->getResources();
    basePacks = files::read_list(resdir/fs::path("config/builtins.list"));
}

void Engine::loadSettings() {
    fs::path settings_file = paths->getSettingsFile();
    if (fs::is_regular_file(settings_file)) {
        logger.info() << "loading settings";
        std::string text = files::read_string(settings_file);
        toml::parse(settingsHandler, settings_file.string(), text);
    }
}

void Engine::loadControls() {
    fs::path controls_file = paths->getControlsFile();
    if (fs::is_regular_file(controls_file)) {
        logger.info() << "loading controls";
        std::string text = files::read_string(controls_file);
        Events::loadBindings(controls_file.u8string(), text);
    }
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
        saveScreenshot();
    }
    if (Events::jpressed(keycode::F11)) {
        settings.display.fullscreen.toggle();
    }
}

void Engine::saveScreenshot() {
    auto image = Window::takeScreenshot();
    image->flipY();
    fs::path filename = paths->getScreenshotFile("png");
    imageio::write(filename.string(), image.get());
    logger.info() << "saved screenshot as " << filename.u8string();
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

        gui->act(delta, Viewport(Window::width, Window::height));
        screen->update(delta);

        if (!Window::isIconified()) {
            renderFrame(batch);
        }
        Window::swapInterval(Window::isIconified() ? 1 : settings.display.vsync.get());

        processPostRunnables();

        Window::swapBuffers();
        Events::pollEvents();
    }
}

void Engine::renderFrame(Batch2D& batch) {
    screen->draw(delta);

    Viewport viewport(Window::width, Window::height);
    DrawContext ctx(nullptr, viewport, &batch);
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

void Engine::saveSettings() {
    logger.info() << "saving settings";
    files::write_string(paths->getSettingsFile(), toml::stringify(settingsHandler));
    logger.info() << "saving bindings";
    files::write_string(paths->getControlsFile(), Events::writeBindings());
}

Engine::~Engine() {
    saveSettings();
    logger.info() << "shutting down";
    if (screen) {
        screen->onEngineShutdown();
        screen.reset();
    }
    content.reset();
    assets.reset();
    interpreter.reset();
    gui.reset();
    logger.info() << "gui finished";
    audio::close();
    scripting::close();
    logger.info() << "scripting finished";
    Window::terminate();
    logger.info() << "engine finished";
}

EngineController* Engine::getController() {
    return controller.get();
}

cmd::CommandsInterpreter* Engine::getCommandsInterpreter() {
    return interpreter.get();
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

    bool threading = false;
    if (threading) {
        auto task = loader.startTask([=](){});
        task->waitForEnd();
    } else {
        while (loader.hasNext()) {
            if (!loader.loadNext()) {
                new_assets.reset();
                throw std::runtime_error("could not to load assets");
            }
        }
    }
    assets.reset(new_assets.release());
}

static void load_configs(const fs::path& root) {
    auto configFolder = root/fs::path("config");
    auto bindsFile = configFolder/fs::path("bindings.toml");
    if (fs::is_regular_file(bindsFile)) {
        Events::loadBindings(
            bindsFile.u8string(), files::read_string(bindsFile)
        );
    }
}

void Engine::loadContent() {
    auto resdir = paths->getResources();
    ContentBuilder contentBuilder;
    corecontent::setup(paths, &contentBuilder);
    paths->setContentPacks(&contentPacks);

    std::vector<std::string> names;
    for (auto& pack : contentPacks) {
        names.push_back(pack.id);
    }
    PacksManager manager = createPacksManager(paths->getWorldFolder());
    manager.scan();
    names = manager.assembly(names);
    contentPacks = manager.getAll(names);

    std::vector<PathsRoot> resRoots;
    for (auto& pack : contentPacks) {
        resRoots.push_back({pack.id, pack.folder});

        ContentLoader loader(&pack);
        loader.load(contentBuilder);

        load_configs(pack.folder);
    } 
    load_configs(paths->getResources());

    content = contentBuilder.build();
    resPaths = std::make_unique<ResPaths>(resdir, resRoots);

    langs::setup(resdir, langs::current->getId(), contentPacks);
    loadAssets();
    onAssetsLoaded();
}

void Engine::resetContent() {
    auto manager = createPacksManager(fs::path());
    manager.scan();
    contentPacks = manager.getAll(basePacks);
    loadContent();
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
    PacksManager manager = createPacksManager(paths->getWorldFolder());
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
    this->screen = std::move(screen);
}

void Engine::setLanguage(std::string locale) {
    langs::setup(paths->getResources(), std::move(locale), contentPacks);
    gui->getMenu()->setPageLoader(menus::create_page_loader(this));
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

std::vector<std::string>& Engine::getBasePacks() {
    return basePacks;
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

void Engine::postRunnable(const runnable& callback) {
    std::lock_guard<std::recursive_mutex> lock(postRunnablesMutex);
    postRunnables.push(callback);
}

SettingsHandler& Engine::getSettingsHandler() {
    return settingsHandler;
}
