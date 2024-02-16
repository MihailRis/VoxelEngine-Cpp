#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include "typedefs.h"
#include "settings.h"

#include "assets/Assets.h"
#include "content/Content.h"
#include "content/ContentPack.h"
#include "files/engine_paths.h"

class Level;
class Screen;
class EnginePaths;
class ResPaths;

namespace fs = std::filesystem;

namespace gui {
	class GUI;
}

class initialize_error : public std::runtime_error {
public:
	initialize_error(const std::string& message) : std::runtime_error(message) {}
};

class Engine {
	std::unique_ptr<Assets> assets = nullptr;
	std::shared_ptr<Screen> screen = nullptr;
    std::vector<ContentPack> contentPacks;
	EngineSettings& settings;
	std::unique_ptr<Content> content = nullptr;
	EnginePaths* paths;
    std::unique_ptr<ResPaths> resPaths = nullptr;

	uint64_t frame = 0;
	double lastTime = 0.0;
	double delta = 0.0;

	std::unique_ptr<gui::GUI> gui;
    
    void updateTimers();
	void updateHotkeys();
public:
	Engine(EngineSettings& settings, EnginePaths* paths);
	~Engine();

    /** 
     * Start main engine input/update/render loop 
     * Automatically sets MenuScreen
     */
	void mainloop();

    /**
     * Set screen (scene).
     * nullptr may be used to delete previous screen before creating new one
     * example:
     *
     *     engine->setScreen(nullptr);
     *     engine->setScreen(std::make_shared<...>(...));
     * 
     * not-null value must be set before next frame
     */
    void setScreen(std::shared_ptr<Screen> screen);
    
    /** 
     * Change locale to specified
     * @param locale isolanguage_ISOCOUNTRY (example: en_US)
     */
	void setLanguage(std::string locale);

    /** 
     * Load all selected content-packs and reload assets 
     */
    void loadContent();
    /**
     * Collect world content-packs and load content
     * @see loadContent
     * @param folder world folder
     */
    void loadWorldContent(const fs::path& folder);

    /**
     * Collect all available content-packs from res/content
     */
	void loadAllPacks();

    /** Get current frame delta-time */
    double getDelta() const;

    /** Get active assets storage instance */
	Assets* getAssets();
    
    /** Get main UI controller */
	gui::GUI* getGUI();

    /** Get writeable engine settings structure instance */
	EngineSettings& getSettings();

    /** Get engine filesystem paths source */
	EnginePaths* getPaths();

    /** Get current Content instance */
	const Content* getContent() const;

    /** Get selected content packs */
    std::vector<ContentPack>& getContentPacks();

    /** Get current screen */
    std::shared_ptr<Screen> getScreen();
};

#endif // SRC_ENGINE_H_
