#pragma once

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "PostRunnables.hpp"
#include "Profiler.hpp"
#include "Time.hpp"
#include "assets/Assets.hpp"
#include "content/ContentPack.hpp"
#include "content/PacksManager.hpp"
#include "content/content_fwd.hpp"
#include "delegates.hpp"
#include "files/engine_paths.hpp"
#include "files/settings_io.hpp"
#include "settings.hpp"
#include "typedefs.hpp"
#include "util/ObjectsKeeper.hpp"

class Level;
class Screen;
class EnginePaths;
class ResPaths;
class EngineController;
class SettingsHandler;
struct EngineSettings;

namespace gui {
    class GUI;
}

namespace cmd {
    class CommandsInterpreter;
}

namespace network {
    class Network;
}

class initialize_error : public std::runtime_error {
public:
    initialize_error(const std::string& message) : std::runtime_error(message) {
    }
};

struct CoreParameters {
    bool headless = false;
    bool testMode = false;
    std::filesystem::path resFolder {"res"};
    std::filesystem::path userFolder {"."};
    std::filesystem::path scriptFile;
};

class Engine : public util::ObjectsKeeper {
    CoreParameters params;
    EngineSettings settings;
    SettingsHandler settingsHandler;
    EnginePaths paths;

    std::unique_ptr<Assets> assets;
    std::shared_ptr<Screen> screen;
    std::vector<ContentPack> contentPacks;
    std::unique_ptr<Content> content;
    std::unique_ptr<ResPaths> resPaths;
    std::unique_ptr<EngineController> controller;
    std::unique_ptr<cmd::CommandsInterpreter> interpreter;
    std::unique_ptr<network::Network> network;
    std::vector<std::string> basePacks;
    std::unique_ptr<gui::GUI> gui;
    PostRunnables postRunnables;
    Time time;
    consumer<std::unique_ptr<Level>> levelConsumer;
    bool quitSignal = false;

    void loadControls();
    void loadSettings();
    void saveSettings();
    void updateHotkeys();
    void loadAssets();
public:
    Engine(CoreParameters coreParameters);
    ~Engine();

    /// @brief Start the engine
    void run();

    void postUpdate();

    void updateFrontend();
    void renderFrame();
    void nextFrame();

    /// @brief Called after assets loading when all engine systems are
    /// initialized
    void onAssetsLoaded();

    /// @brief Set screen (scene).
    /// nullptr may be used to delete previous screen before creating new one,
    /// not-null value must be set before next frame
    /// @param screen nullable screen
    void setScreen(std::shared_ptr<Screen> screen);

    /// @brief Change locale to specified
    /// @param locale isolanguage_ISOCOUNTRY (example: en_US)
    void setLanguage(std::string locale);

    /// @brief Load all selected content-packs and reload assets
    void loadContent();

    void resetContent();

    /// @brief Collect world content-packs and load content
    /// @see loadContent
    /// @param folder world folder
    void loadWorldContent(const fs::path& folder);

    /// @brief Collect all available content-packs from res/content
    void loadAllPacks();

    /// @brief Get active assets storage instance
    Assets* getAssets();

    /// @brief Get main UI controller
    gui::GUI* getGUI();

    /// @brief Get writeable engine settings structure instance
    EngineSettings& getSettings();

    /// @brief Get engine filesystem paths source
    EnginePaths& getPaths();

    /// @brief Get engine resource paths controller
    ResPaths* getResPaths();

    void onWorldOpen(std::unique_ptr<Level> level);
    void onWorldClosed();

    void quit();

    bool isQuitSignal() const;

    /// @brief Get current Content instance
    const Content* getContent() const;

    /// @brief Get selected content packs
    std::vector<ContentPack>& getContentPacks();

    std::vector<ContentPack> getAllContentPacks();

    std::vector<std::string>& getBasePacks();

    /// @brief Get current screen
    std::shared_ptr<Screen> getScreen();

    /// @brief Enqueue function call to the end of current frame in draw thread
    void postRunnable(const runnable& callback) {
        postRunnables.postRunnable(callback);
    }

    void saveScreenshot();

    EngineController* getController();
    cmd::CommandsInterpreter* getCommandsInterpreter();

    PacksManager createPacksManager(const fs::path& worldFolder);

    void setLevelConsumer(consumer<std::unique_ptr<Level>> levelConsumer);

    SettingsHandler& getSettingsHandler();

    network::Network& getNetwork();

    Time& getTime();

    const CoreParameters& getCoreParameters() const;

    bool isHeadless() const;
};
