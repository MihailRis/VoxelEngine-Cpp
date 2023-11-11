#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <string>
#include <memory>
#include <stdexcept>
#include "typedefs.h"
#include "settings.h"

class Assets;
class Level;

namespace gui {
	class GUI;
}

class initialize_error : public std::runtime_error {
public:
	initialize_error(const std::string& message) : std::runtime_error(message) {}
};

class Engine {
	Assets* assets;
	Level* level;
	EngineSettings settings;

	uint64_t frame = 0;
	double lastTime = 0.0;
	double delta = 0.0;
	bool occlusion = true;

	gui::GUI* gui;
public:
	Engine(const EngineSettings& settings);
	~Engine();

	void updateTimers();
	void updateHotkeys();
	void mainloop();
};

#endif // SRC_ENGINE_H_