#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <delegates.hpp>
#include <typedefs.hpp>

#include <assets/Assets.hpp>
#include <content/content_fwd.hpp>
#include <content/ContentPack.hpp>
#include <content/PacksManager.hpp>
#include <files/engine_paths.hpp>
#include <util/ObjectsKeeper.hpp>

#include <filesystem>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>
#include <mutex>

class Level;
class Screen;
class EnginePaths;
class ResPaths;
class Batch2D;
class EngineController;
class SettingsHandler;
struct EngineSettings;

namespace fs = std::filesystem;

namespace gui {
    class GUI;
}

namespace cmd {
    class CommandsInterpreter;
}

class initialize_error : public std::runtime_error {
public:
    initialize_error(const std::string& message) : std::runtime_error(message) {}
};

class Engine : public util::ObjectsKeeper {
    EngineSettings& settings;
    SettingsHandler& settingsHandler;
    EnginePaths* paths;

    std::unique_ptr<Assets> assets;
    std::shared_ptr<Screen> screen;
    std::vector<ContentPack> contentPacks;
    std::unique_ptr<Content> content;
    std::unique_ptr<ResPaths> resPaths;
    std::queue<runnable> postRunnables;
    std::recursive_mutex postRunnablesMutex;
    std::unique_ptr<EngineController> controller;
    std::unique_ptr<cmd::CommandsInterpreter> interpreter;
    std::vector<std::string> basePacks;

    uint64_t frame = 0;
    double lastTime = 0.0;
    double delta = 0.0;

    std::unique_ptr<gui::GUI> gui;
    
    void loadControls();
    void loadSettings();
    void saveSettings();
    void updateTimers();
    void updateHotkeys();
    void renderFrame(Batch2D& batch);
    void processPostRunnables();
    void loadAssets();
public:
    Engine(EngineSettings& settings, SettingsHandler& settingsHandler, EnginePaths* paths);
    ~Engine();
 
    /// @brief Start main engine input/update/render loop. 
    /// Automatically sets MenuScreen
    void mainloop();

    /// @brief Called after assets loading when all engine systems are initialized
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

    /// @brief Get current frame delta-time
    double getDelta() const;

    /// @brief Get active assets storage instance
    Assets* getAssets();
    
    /// @brief Get main UI controller
    gui::GUI* getGUI();

    /// @brief Get writeable engine settings structure instance
    EngineSettings& getSettings();

    /// @brief Get engine filesystem paths source
    EnginePaths* getPaths();

    /// @brief Get engine resource paths controller
    ResPaths* getResPaths();

    /// @brief Get current Content instance
    const Content* getContent() const;

    /// @brief Get selected content packs
    std::vector<ContentPack>& getContentPacks();

    std::vector<std::string>& getBasePacks();

    /// @brief Get current screen
    std::shared_ptr<Screen> getScreen();

    /// @brief Enqueue function call to the end of current frame in draw thread
    void postRunnable(const runnable& callback);

    void saveScreenshot();

    EngineController* getController();
    cmd::CommandsInterpreter* getCommandsInterpreter();

    PacksManager createPacksManager(const fs::path& worldFolder);

    SettingsHandler& getSettingsHandler();
};

#endif // ENGINE_HPP_
