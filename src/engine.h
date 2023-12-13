#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include "typedefs.h"
#include "settings.h"

#include "content/ContentPack.h"

class Assets;
class Level;
class Screen;
class Content;
class EnginePaths;

namespace gui {
	class GUI;
}

class initialize_error : public std::runtime_error {
public:
	initialize_error(const std::string& message) : std::runtime_error(message) {}
};

class Engine {
	Assets* assets;
	std::shared_ptr<Screen> screen = nullptr;
    std::vector<ContentPack> contentPacks;
	EngineSettings& settings;
	Content* content;
	EnginePaths* paths;

	uint64_t frame = 0;
	double lastTime = 0.0;
	double delta = 0.0;

	gui::GUI* gui;
public:
	Engine(EngineSettings& settings, EnginePaths* paths, Content* content);
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
};

#endif // SRC_ENGINE_H_