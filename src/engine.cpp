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
#include "graphics/Shader.h"
#include "graphics/ImageData.h"
#include "frontend/gui/GUI.h"
#include "frontend/screens.h"
#include "util/platform.h"

#include "coders/json.h"
#include "coders/png.h"
#include "coders/GLSLExtension.h"
#include "files/files.h"
#include "files/engine_paths.h"
#include "graphics-base/IShader.h"
#include "graphics-vk/Batch2D.h"
#include "graphics-vk/VulkanContext.h"
#include "graphics-vk/texture/ImageCube.h"

using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::filesystem::path;
using glm::vec3;
using gui::GUI;

Engine::Engine(EngineSettings& settings, EnginePaths* paths, Content* content)
	   : settings(settings), content(content), paths(paths) {
	if (Window::initialize(settings.display)){
		throw initialize_error("could not initialize window");
	}
	Shader::preprocessor->setLibFolder(paths->getResources()/path("shaders/lib"));

	vulkan::VulkanContext::initialize();

	assets = new Assets();
	std::cout << "-- loading assets" << std::endl;
	AssetsLoader loader(assets, paths->getResources());
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
		path filename = paths->getScreenshotFile("png");
		png::write_image(filename.string(), image.get());
		std::cout << "saved screenshot as " << filename << std::endl;
	}
	if (Events::jpressed(keycode::F11)) {
		Window::toggleFullscreen();
	}
}

void Engine::mainloop() {
	setScreen(std::make_shared<MenuScreen>(this));
	
	std::cout << "-- preparing systems" << std::endl;

	vulkan::Batch2D batch(5000);
	lastTime = Window::time();

	while (!Window::isShouldClose()){
		assert(screen != nullptr);
		updateTimers();
		updateHotkeys();

		gui->act(delta);
		screen->update(delta);

		screen->draw(delta);
		gui->draw(&batch, assets);

		vulkan::VulkanContext::get().draw();

		// Window::swapInterval(settings.display.swapInterval);
		Window::swapBuffers();
		Events::pullEvents();
	}

	vulkan::VulkanContext::waitIdle();
}

Engine::~Engine() {
	screen = nullptr;
	delete gui;

	Audio::finalize();

	std::cout << "-- shutting down" << std::endl;
	delete assets;
	vulkan::VulkanContext::finalize();
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

const Content* Engine::getContent() const {
	return content;
}

EnginePaths* Engine::getPaths() {
	return paths;
}