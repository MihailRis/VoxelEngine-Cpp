// Install dependencies:
// sudo apt install libgl-dev libglew-dev libglfw3-dev libpng-dev libglm-dev
#include <iostream>
#include <cmath>
#include <stdint.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <ctime>
#include <exception>

// GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "audio/Audio.h"
#include "voxels/Chunk.h"
#include "voxels/Chunks.h"
#include "voxels/ChunksController.h"
#include "voxels/ChunksLoader.h"
#include "objects/Player.h"
#include "world/Level.h"
#include "world/World.h"
#include "declarations.h"
#include "Assets.h"
#include "AssetsLoader.h"
#include "world_render.h"
#include "hud_render.h"


class initialize_error : public std::runtime_error {
	initialize_error(const std::string& message) : std::runtime_error(message) {}
};

struct EngineSettings {
	int displayWidth;
	int displayHeight;
	const char* title;
};


class Engine {
	Assets* assets;
	Level* level;

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
	Window::initialize(settings.displayWidth, settings.displayHeight, settings.title);
	Events::initialize();

	assets = new Assets();
	std::cout << "-- loading assets" << std::endl;
	AssetsLoader loader(assets);
	AssetsLoader::createDefaults(loader);
	initialize_assets(&loader);
	while (loader.hasNext()) {
		if (!loader.loadNext()) {
			delete assets;
			Window::terminate();
			throw std::runtime_error("could not to initialize assets");
		}
	}
	std::cout << "-- loading world" << std::endl;
	vec3 playerPosition = vec3(0, 64, 0);
	Camera* camera = new Camera(playerPosition, radians(90.0f));
	World* world = new World("world-1", "world/", 42);
	Player* player = new Player(playerPosition, 4.0f, camera);
	level = world->loadLevel(player);

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
		for (unsigned i = 0; i < level->chunks->volume; i++) {
			Chunk* chunk = level->chunks->chunks[i];
			if (chunk != nullptr && chunk->isReady()) {
				chunk->setModified(true);
			}
		}
	}
}

void Engine::mainloop() {
	Camera* camera = level->player->camera;
	std::cout << "-- preparing systems" << std::endl;
	World* world = level->world;
	WorldRenderer worldRenderer(level, assets);
	HudRenderer hud;
	lastTime = glfwGetTime();

	Window::swapInterval(1);
	while (!Window::isShouldClose()){
		updateTimers();
		updateHotkeys();

		level->update(delta, Events::_cursor_locked);
		int freeLoaders = level->chunksController->countFreeLoaders();
		for (int i = 0; i < freeLoaders; i++)
			level->chunksController->_buildMeshes();
		freeLoaders = level->chunksController->countFreeLoaders();
		for (int i = 0; i < freeLoaders; i++)
			level->chunksController->calculateLights();
		freeLoaders = level->chunksController->countFreeLoaders();
		for (int i = 0; i < freeLoaders; i++)
			level->chunksController->loadVisible(world->wfile);

		worldRenderer.draw(camera, occlusion);
		hud.draw(level, assets);
		if (level->player->debug) {
			hud.drawDebug(level, assets, 1 / delta, occlusion);
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
	Events::finalize();
	Window::terminate();
}

int main() {
	setup_definitions();

	try {
		Engine engine(EngineSettings{ 1280, 720, "VoxelEngine-Cpp v13" });
		engine.mainloop();
	}
	catch (const initialize_error& err) {
		std::cerr << "could not to initialize engine" << std::endl;
		std::cerr << err.what() << std::endl;
	}

	return 0;
}
