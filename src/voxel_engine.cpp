#include <iostream>
#include <cmath>
#include <stdint.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>
#include <ctime>
#include <exception>
#include <filesystem>

// GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "coders/json.h"
#include "files/files.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "audio/Audio.h"
#include "voxels/Chunk.h"
#include "voxels/Chunks.h"
#include "voxels/ChunksController.h"
#include "voxels/ChunksStorage.h"
#include "objects/Player.h"
#include "world/Level.h"
#include "world/World.h"
#include "definitions.h"
#include "assets/Assets.h"
#include "assets/AssetsLoader.h"
#include "frontend/world_render.h"
#include "frontend/hud_render.h"

#define SETTINGS_FILE "settings.json"

using std::shared_ptr;


class initialize_error : public std::runtime_error {
public:
	initialize_error(const std::string& message) : std::runtime_error(message) {}
};

struct EngineSettings {
    /* Window width (pixels) */
	int displayWidth;
	/* Window height (pixels) */
	int displayHeight;
	/* Anti-aliasing samples */
	int displaySamples;
	/* Window title */
	const char* displayTitle;
	/* Max milliseconds that engine uses for chunks loading only */
	uint chunksLoadSpeed;
	/* Radius of chunks loading zone (chunk is unit) */
	uint chunksLoadDistance;
	/* Buffer zone where chunks are not unloading (chunk is unit)*/
	uint chunksPadding;
};

void save_settings(EngineSettings& settings, std::string filename);

class Engine {
	Assets* assets;
	Level* level;
	EngineSettings settings;

	uint64_t frame = 0;
	float lastTime = 0.0f;
	float delta = 0.0f;
	bool occlusion = true;
public:
	Engine(const EngineSettings& settings);
	~Engine();

	void updateTimers();
	void updateHotkeys();
	void mainloop();
};

Engine::Engine(const EngineSettings& settings) {
    this->settings = settings;
    
	Window::initialize(settings.displayWidth, 
	                   settings.displayHeight, 
	                   settings.displayTitle, 
	                   settings.displaySamples);

	assets = new Assets();
	std::cout << "-- loading assets" << std::endl;
	AssetsLoader loader(assets);
	AssetsLoader::createDefaults(loader);
	AssetsLoader::addDefaults(loader);
	while (loader.hasNext()) {
		if (!loader.loadNext()) {
			delete assets;
			Window::terminate();
			throw initialize_error("could not to initialize assets");
		}
	}
	std::cout << "-- loading world" << std::endl;
	vec3 playerPosition = vec3(0, 64, 0);
	Camera* camera = new Camera(playerPosition, radians(90.0f));
	World* world = new World("world-1", "world/", 42);
	Player* player = new Player(playerPosition, 4.0f, camera);
	level = world->loadLevel(player, settings.chunksLoadDistance, settings.chunksPadding);

	std::cout << "-- initializing finished" << std::endl;

	Audio::initialize();
}

void Engine::updateTimers() {
	frame++;
	float currentTime = glfwGetTime();
	delta = currentTime - lastTime;
	lastTime = currentTime;
}

void Engine::updateHotkeys() {
	if (Events::jpressed(GLFW_KEY_ESCAPE)) {
		Window::setShouldClose(true);
	}
	if (Events::jpressed(GLFW_KEY_TAB) || Events::jpressed(GLFW_KEY_E)) {
		Events::toggleCursor();
	}
	if (Events::jpressed(GLFW_KEY_O)) {
		occlusion = !occlusion;
	}
	if (Events::jpressed(GLFW_KEY_F3)) {
		level->player->debug = !level->player->debug;
	}
	if (Events::jpressed(GLFW_KEY_F5)) {
		for (uint i = 0; i < level->chunks->volume; i++) {
			shared_ptr<Chunk> chunk = level->chunks->chunks[i];
			if (chunk != nullptr && chunk->isReady()) {
				chunk->setModified(true);
			}
		}
	}
}

void Engine::mainloop() {
	Camera* camera = level->player->camera;
	std::cout << "-- preparing systems" << std::endl;
	WorldRenderer worldRenderer(level, assets);
	HudRenderer hud(assets);
	lastTime = glfwGetTime();

	Window::swapInterval(1);
	while (!Window::isShouldClose()){
		updateTimers();
		updateHotkeys();

		level->update(delta, Events::_cursor_locked);
		level->chunksController->update(settings.chunksLoadSpeed);

		worldRenderer.draw(camera, occlusion);
		hud.draw(level);
		if (level->player->debug) {
			hud.drawDebug(level, 1 / delta, occlusion);
		}

		Window::swapBuffers();
		Events::pullEvents();
	}
}

Engine::~Engine() {
	Audio::finalize();

	World* world = level->world;

	std::cout << "-- saving world" << std::endl;
	world->write(level);

	delete level;
	delete world;

	std::cout << "-- shutting down" << std::endl;
	delete assets;
	Window::terminate();
}

void load_settings(EngineSettings& settings, std::string filename) {
	std::string source = files::read_string(filename);
	std::unique_ptr<json::JObject> obj(json::parse(filename, source));
	obj->num("display-width", settings.displayWidth);
	obj->num("display-height", settings.displayHeight);
	obj->num("display-samples", settings.displaySamples);
	obj->num("chunks-load-distance", settings.chunksLoadDistance);
	obj->num("chunks-load-speed", settings.chunksLoadSpeed);
	obj->num("chunks-padding", settings.chunksPadding);
}

void save_settings(EngineSettings& settings, std::string filename) {
	json::JObject obj;
	obj.put("display-width", settings.displayWidth);
	obj.put("display-height", settings.displayHeight);
	obj.put("display-samples", settings.displaySamples);
	obj.put("chunks-load-distance", settings.chunksLoadDistance);
	obj.put("chunks-load-speed", settings.chunksLoadSpeed);
	obj.put("chunks-padding", settings.chunksPadding);
	files::write_string(filename, json::stringify(&obj, true, "  "));
}

int main() {
	setup_definitions();
	try {
	    EngineSettings settings;
	    settings.displayWidth = 1280;
	    settings.displayHeight = 720;
	    settings.displaySamples = 0; // верну мультисемплинг позже
	    settings.displayTitle = "VoxelEngine-Cpp v0.13";
	    settings.chunksLoadSpeed = 10;
	    settings.chunksLoadDistance = 12;
	    settings.chunksPadding = 2;

		if (std::filesystem::is_regular_file(SETTINGS_FILE)) {
			std::cout << "-- loading settings" << std::endl;
			load_settings(settings, SETTINGS_FILE);
		} else {
			save_settings(settings, SETTINGS_FILE);
		}
		Engine engine(settings);
		engine.mainloop();
	}
	catch (const initialize_error& err) {
		std::cerr << "could not to initialize engine" << std::endl;
		std::cerr << err.what() << std::endl;
	}
	return 0;
}
