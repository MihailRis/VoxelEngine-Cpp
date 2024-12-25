#include "engine.hpp"

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include "debug/Logger.hpp"
#include "assets/AssetsLoader.hpp"
#include "audio/audio.hpp"
#include "coders/GLSLExtension.hpp"
#include "coders/imageio.hpp"
#include "coders/json.hpp"
#include "coders/toml.hpp"
#include "coders/commons.hpp"
#include "content/Content.hpp"
#include "content/ContentBuilder.hpp"
#include "content/ContentLoader.hpp"
#include "core_defs.hpp"
#include "files/files.hpp"
#include "frontend/locale.hpp"
#include "frontend/menu.hpp"
#include "frontend/screens/Screen.hpp"
#include "graphics/render/ModelsGenerator.hpp"
#include "graphics/core/DrawContext.hpp"
#include "graphics/core/ImageData.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/ui/GUI.hpp"
#include "objects/rigging.hpp"
#include "logic/EngineController.hpp"
#include "logic/CommandsInterpreter.hpp"
#include "logic/scripting/scripting.hpp"
#include "network/Network.hpp"
#include "util/listutil.hpp"
#include "util/platform.hpp"
#include "window/Camera.hpp"
#include "window/Events.hpp"
#include "window/input.hpp"
#include "window/Window.hpp"
#include "world/Level.hpp"
#include "Mainloop.hpp"
#include "ServerMainloop.hpp"

#include <iostream>
#include <assert.h>
#include <glm/glm.hpp>
#include <unordered_set>
#include <functional>
#include <utility>

static debug::Logger logger("engine");

namespace fs = std::filesystem;

static void create_channel(Engine* engine, std::string name, NumberSetting& setting) {
    if (name != "master") {
        audio::create_channel(name);
    }
    engine->keepAlive(setting.observe([=](auto value) {
        audio::get_channel(name)->setVolume(value*value);
    }, true));
}

static std::unique_ptr<ImageData> load_icon(const fs::path& resdir) {
    try {
        auto file = resdir / fs::u8path("textures/misc/icon.png");
        if (fs::exists(file)) {
            return imageio::read(file);
        }
    } catch (const std::exception& err) {
        logger.error() << "could not load window icon: " << err.what();
    }
    return nullptr;
}

Engine::Engine(CoreParameters coreParameters)
    : params(std::move(coreParameters)),
      settings(),
      settingsHandler({settings}),
      interpreter(std::make_unique<cmd::CommandsInterpreter>()),
      network(network::Network::create(settings.network)) {
    logger.info() << "engine version: " << ENGINE_VERSION_STRING;
    if (params.headless) {
        logger.info() << "headless mode is enabled";
    }
    paths.setResourcesFolder(params.resFolder);
    paths.setUserFilesFolder(params.userFolder);
    paths.prepare();
    if (!params.scriptFile.empty()) {
        paths.setScriptFolder(params.scriptFile.parent_path());
    }
    loadSettings();

    auto resdir = paths.getResourcesFolder();

    controller = std::make_unique<EngineController>(*this);
    if (!params.headless) {
        if (Window::initialize(&settings.display)){
            throw initialize_error("could not initialize window");
        }
        time.set(Window::time());
        if (auto icon = load_icon(resdir)) {
            icon->flipY();
            Window::setIcon(icon.get());
        }
        loadControls();

        gui = std::make_unique<gui::GUI>();
        if (ENGINE_DEBUG_BUILD) {
            menus::create_version_label(*this);
        }
    }
    audio::initialize(settings.audio.enabled.get() && !params.headless);
    create_channel(this, "master", settings.audio.volumeMaster);
    create_channel(this, "regular", settings.audio.volumeRegular);
    create_channel(this, "music", settings.audio.volumeMusic);
    create_channel(this, "ambient", settings.audio.volumeAmbient);
    create_channel(this, "ui", settings.audio.volumeUI);

    bool langNotSet = settings.ui.language.get() == "auto";
    if (langNotSet) {
        settings.ui.language.set(langs::locale_by_envlocale(
            platform::detect_locale(),
            paths.getResourcesFolder()
        ));
    }
    keepAlive(settings.ui.language.observe([this](auto lang) {
        setLanguage(lang);
    }, true));
    
    scripting::initialize(this);
    basePacks = files::read_list(resdir/fs::path("config/builtins.list"));
}

void Engine::loadSettings() {
    fs::path settings_file = paths.getSettingsFile();
    if (fs::is_regular_file(settings_file)) {
        logger.info() << "loading settings";
        std::string text = files::read_string(settings_file);
        try {
            toml::parse(settingsHandler, settings_file.string(), text);
        } catch (const parsing_error& err) {
            logger.error() << err.errorLog();
            throw;
        }
    }
}

void Engine::loadControls() {
    fs::path controls_file = paths.getControlsFile();
    if (fs::is_regular_file(controls_file)) {
        logger.info() << "loading controls";
        std::string text = files::read_string(controls_file);
        Events::loadBindings(controls_file.u8string(), text, BindType::BIND);
    }
}

void Engine::onAssetsLoaded() {
    assets->setup();
    gui->onAssetsLoad(assets.get());
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
    fs::path filename = paths.getNewScreenshotFile("png");
    imageio::write(filename.string(), image.get());
    logger.info() << "saved screenshot as " << filename.u8string();
}

void Engine::run() {
    if (params.headless) {
        ServerMainloop(*this).run();
    } else {
        Mainloop(*this).run();
    }
}

void Engine::postUpdate() {
    network->update();
    processPostRunnables();
}

void Engine::updateFrontend() {
    double delta = time.getDelta();
    updateHotkeys();
    audio::update(delta);
    gui->act(delta, Viewport(Window::width, Window::height));
    screen->update(delta);
}

void Engine::nextFrame() {
    Window::setFramerate(
        Window::isIconified() && settings.display.limitFpsIconified.get()
            ? 20
            : settings.display.framerate.get()
    );
    Window::swapBuffers();
    Events::pollEvents();
}

void Engine::renderFrame() {
    screen->draw(time.getDelta());

    Viewport viewport(Window::width, Window::height);
    DrawContext ctx(nullptr, viewport, nullptr);
    gui->draw(ctx, *assets);
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
    files::write_string(paths.getSettingsFile(), toml::stringify(settingsHandler));
    if (!params.headless) {
        logger.info() << "saving bindings";
        files::write_string(paths.getControlsFile(), Events::writeBindings());
    }
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
    if (gui) {
        gui.reset();
        logger.info() << "gui finished";
    }
    audio::close();
    network.reset();
    clearKeepedObjects();
    scripting::close();
    logger.info() << "scripting finished";
    if (!params.headless) {
        Window::terminate();
        logger.info() << "window closed";
    }
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
        paths.getUserFilesFolder()/fs::path("content"),
        paths.getResourcesFolder()/fs::path("content")
    });
    return manager;
}

void Engine::setLevelConsumer(consumer<std::unique_ptr<Level>> levelConsumer) {
    this->levelConsumer = std::move(levelConsumer);
}

void Engine::loadAssets() {
    logger.info() << "loading assets";
    Shader::preprocessor->setPaths(resPaths.get());

    auto new_assets = std::make_unique<Assets>();
    AssetsLoader loader(new_assets.get(), resPaths.get());
    AssetsLoader::addDefaults(loader, content.get());

    // no need
    // correct log messages order is more useful
    bool threading = false; // look at two upper lines
    if (threading) {
        auto task = loader.startTask([=](){});
        task->waitForEnd();
    } else {
        try {
            while (loader.hasNext()) {
                loader.loadNext();
            }
        } catch (const assetload::error& err) {
            new_assets.reset();
            throw;
        }
    }
    assets = std::move(new_assets);

    if (content == nullptr) {
        return;
    }
    for (auto& [name, def] : content->blocks.getDefs()) {
        if (def->model == BlockModel::custom && def->modelName.empty()) {
            assets->store(
                std::make_unique<model::Model>(
                    ModelsGenerator::loadCustomBlockModel(
                        def->customModelRaw, *assets, !def->shadeless
                    )
                ),
                name + ".model"
            );
            def->modelName = def->name + ".model";
        }
    }
    for (auto& [name, def] : content->items.getDefs()) {
        assets->store(
            std::make_unique<model::Model>(
                ModelsGenerator::generate(*def, *content, *assets)
            ),
            name + ".model"
        );
    }
}

static void load_configs(const fs::path& root) {
    auto configFolder = root/fs::path("config");
    auto bindsFile = configFolder/fs::path("bindings.toml");
    if (fs::is_regular_file(bindsFile)) {
        Events::loadBindings(
            bindsFile.u8string(), files::read_string(bindsFile), BindType::BIND
        );
    }
}

void Engine::loadContent() {
    scripting::cleanup();

    auto resdir = paths.getResourcesFolder();

    std::vector<std::string> names;
    for (auto& pack : contentPacks) {
        names.push_back(pack.id);
    }

    ContentBuilder contentBuilder;
    corecontent::setup(paths, contentBuilder);

    paths.setContentPacks(&contentPacks);
    PacksManager manager = createPacksManager(paths.getCurrentWorldFolder());
    manager.scan();
    names = manager.assembly(names);
    contentPacks = manager.getAll(names);

    auto corePack = ContentPack::createCore(paths);

    // Setup filesystem entry points
    std::vector<PathsRoot> resRoots {
        {"core", corePack.folder}
    };
    for (auto& pack : contentPacks) {
        resRoots.push_back({pack.id, pack.folder});
    }
    resPaths = std::make_unique<ResPaths>(resdir, resRoots);

    // Load content
    {
        ContentLoader(&corePack, contentBuilder, *resPaths).load();
        load_configs(corePack.folder);
    }
    for (auto& pack : contentPacks) {
        ContentLoader(&pack, contentBuilder, *resPaths).load();
        load_configs(pack.folder);
    }
    content = contentBuilder.build();
    interpreter->reset();
    scripting::on_content_load(content.get());

    ContentLoader::loadScripts(*content);

    langs::setup(resdir, langs::current->getId(), contentPacks);
    if (!isHeadless()) {
        loadAssets();
        onAssetsLoaded();
    }
}

void Engine::resetContent() {
    scripting::cleanup();
    auto resdir = paths.getResourcesFolder();
    std::vector<PathsRoot> resRoots;
    {
        auto pack = ContentPack::createCore(paths);
        resRoots.push_back({"core", pack.folder});
        load_configs(pack.folder);
    }
    auto manager = createPacksManager(fs::path());
    manager.scan();
    for (const auto& pack : manager.getAll(basePacks)) {
        resRoots.push_back({pack.id, pack.folder});
    }
    resPaths = std::make_unique<ResPaths>(resdir, resRoots);
    contentPacks.clear();
    content.reset();

    langs::setup(resdir, langs::current->getId(), contentPacks);
    loadAssets();
    onAssetsLoaded();

    contentPacks = manager.getAll(basePacks);
}

void Engine::loadWorldContent(const fs::path& folder) {
    contentPacks.clear();
    auto packNames = ContentPack::worldPacksList(folder);
    PacksManager manager;
    manager.setSources({
        folder/fs::path("content"),
        paths.getUserFilesFolder()/fs::path("content"),
        paths.getResourcesFolder()/fs::path("content")
    });
    manager.scan();
    contentPacks = manager.getAll(manager.assembly(packNames));
    paths.setCurrentWorldFolder(folder);
    loadContent();
}

void Engine::loadAllPacks() {
    PacksManager manager = createPacksManager(paths.getCurrentWorldFolder());
    manager.scan();
    auto allnames = manager.getAllNames();
    contentPacks = manager.getAll(manager.assembly(allnames));
}

void Engine::setScreen(std::shared_ptr<Screen> screen) {
    // reset audio channels (stop all sources)
    audio::reset_channel(audio::get_channel_index("regular"));
    audio::reset_channel(audio::get_channel_index("ambient"));
    this->screen = std::move(screen);
}

void Engine::setLanguage(std::string locale) {
    langs::setup(paths.getResourcesFolder(), std::move(locale), contentPacks);
    if (gui) {
        gui->getMenu()->setPageLoader(menus::create_page_loader(*this));
    }
}

void Engine::onWorldOpen(std::unique_ptr<Level> level) {
    logger.info() << "world open";
    levelConsumer(std::move(level));
}

void Engine::onWorldClosed() {
    logger.info() << "world closed";
    levelConsumer(nullptr);
}

void Engine::quit() {
    quitSignal = true;
    if (!isHeadless()) {
        Window::setShouldClose(true);
    }
}

bool Engine::isQuitSignal() const {
    return quitSignal;
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

std::vector<ContentPack> Engine::getAllContentPacks() {
    auto packs = getContentPacks();
    packs.insert(packs.begin(), ContentPack::createCore(paths));
    return packs;
}

std::vector<ContentPack>& Engine::getContentPacks() {
    return contentPacks;
}

std::vector<std::string>& Engine::getBasePacks() {
    return basePacks;
}

EnginePaths& Engine::getPaths() {
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

network::Network& Engine::getNetwork() {
    return *network;
}

Time& Engine::getTime() {
    return time;
}

const CoreParameters& Engine::getCoreParameters() const {
    return params;
}

bool Engine::isHeadless() const {
    return params.headless;
}
