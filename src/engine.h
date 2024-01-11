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
public:
	Engine(EngineSettings& settings, EnginePaths* paths);
	~Engine();

	void updateTimers();
	void updateHotkeys();
	void mainloop();

	Assets* getAssets();
	gui::GUI* getGUI();
	EngineSettings& getSettings();
	void setScreen(std::shared_ptr<Screen> screen);
	EnginePaths* getPaths();
	const Content* getContent() const;
    std::vector<ContentPack>& getContentPacks();
	void setLanguage(std::string locale);
    void loadContent();
    void loadWorldContent(const fs::path& folder);
	void loadAllPacks();
};

#endif // SRC_ENGINE_H_