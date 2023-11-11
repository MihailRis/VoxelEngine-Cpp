#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <string>
#include <memory>
#include <stdexcept>
#include "typedefs.h"

class Assets;
class Level;

namespace gui {
	class GUI;
}

struct EngineSettings {
    /* Window width (pixels) */
	int displayWidth;
	/* Window height (pixels) */
	int displayHeight;
	/* Anti-aliasing samples */
	int displaySamples;
	/* GLFW swap interval value, 0 - unlimited fps, 1 - vsync*/
	int displaySwapInterval;
	/* Window title */
	const char* displayTitle;
	/* Max milliseconds that engine uses for chunks loading only */
	uint chunksLoadSpeed;
	/* Radius of chunks loading zone (chunk is unit) */
	uint chunksLoadDistance;
	/* Buffer zone where chunks are not unloading (chunk is unit)*/
	uint chunksPadding;
	/* Fog opacity is calculated as `pow(depth*k, fogCurve)` where k depends on chunksLoadDistance.
	   Use values in range [1.0 - 2.0] where 1.0 is linear, 2.0 is quadratic
	*/
	float fogCurve;
};

void load_settings(EngineSettings& settings, std::string filename);
void save_settings(EngineSettings& settings, std::string filename);

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