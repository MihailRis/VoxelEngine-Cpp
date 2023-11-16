#include "engine.h"

#include <memory>
#include <iostream>
#include <assert.h>
#include <glm/glm.hpp>
#include <filesystem>
#define GLEW_STATIC

#include "audio/Audio.h"
#include "assets/Assets.h"
#include "assets/AssetsLoader.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "window/input.h"
#include "graphics/Batch2D.h"
#include "graphics/ImageData.h"
#include "world/World.h"
#include "world/Level.h"
#include "voxels/Chunk.h"
#include "voxels/Chunks.h"
#include "voxels/ChunksController.h"
#include "voxels/ChunksStorage.h"
#include "objects/Player.h"
#include "frontend/world_render.h"
#include "frontend/hud.h"
#include "frontend/gui/GUI.h"
#include "frontend/screens.h"
#include "util/platform.h"

#include "coders/json.h"
#include "coders/png.h"
#include "files/files.h"
#include "files/engine_files.h"

using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::filesystem::path;
using glm::vec3;
using gui::GUI;

Engine::Engine(const EngineSettings& settings_) {
    this->settings = settings_;
    
	Window::initialize(settings.display);

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
	Audio::initialize();
	gui = new GUI();
	std::cout << "-- initializing finished" << std::endl;

	setScreen(shared_ptr<Screen>(new MenuScreen(this)));
}

void Engine::updateTimers() {
	frame++;
	double currentTime = Window::time();
	delta = currentTime - lastTime;
	lastTime = currentTime;
}

void Engine::updateHotkeys() {
	if (Events::jpressed(keycode::F2)) {
		unique_ptr<ImageData> image(Window::takeScreenshot());
		image->flipY();
		path filename = enginefs::get_screenshot_file("png");
		png::write_image(filename, image.get());
		std::cout << "saved screenshot as " << filename << std::endl;
	}
}

void Engine::mainloop() {
	std::cout << "-- preparing systems" << std::endl;

	Batch2D batch(1024);
	lastTime = Window::time();

	while (!Window::isShouldClose()){
		assert(screen != nullptr);
		updateTimers();
		updateHotkeys();

		screen->update(delta);
		screen->draw(delta);
		gui->act(delta);
		gui->draw(&batch, assets);

		Window::swapBuffers();
		Events::pullEvents();
	}
}

Engine::~Engine() {
	screen = nullptr;
	delete gui;

	Audio::finalize();

	std::cout << "-- shutting down" << std::endl;
	delete assets;
	Window::terminate();
	std::cout << "-- engine finished" << std::endl;
}

GUI* Engine::getGUI() {
	return gui;
}

EngineSettings& Engine::getSettings() {
	return settings;
}

Assets* Engine::getAssets() {
	return assets;
}

void Engine::setScreen(shared_ptr<Screen> screen) {
	this->screen = screen;
}