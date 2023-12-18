#include "engine.h"

#include <memory>
#include <iostream>
#include <assert.h>
#include <vector>
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
#include "frontend/menu.h"
#include "util/platform.h"

#include "coders/json.h"
#include "coders/png.h"
#include "coders/GLSLExtension.h"
#include "files/files.h"
#include "files/engine_paths.h"

#include "content/Content.h"
#include "content/ContentPack.h"
#include "content/ContentLoader.h"
#include "frontend/locale/langs.h"

#include "definitions.h"

using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::vector;
using std::filesystem::path;
using glm::vec3;
using gui::GUI;

Engine::Engine(EngineSettings& settings, EnginePaths* paths) 
	   : settings(settings), paths(paths) {    
	if (Window::initialize(settings.display)){
		throw initialize_error("could not initialize window");
	}

    auto resdir = paths->getResources();

	std::cout << "-- loading assets" << std::endl;
    std::vector<path> roots {resdir};
    resPaths.reset(new ResPaths(resdir, roots));
    assets.reset(new Assets());
	AssetsLoader loader(assets.get(), resPaths.get());
	AssetsLoader::createDefaults(loader);
	AssetsLoader::addDefaults(loader);

    Shader::preprocessor->setPaths(resPaths.get());
	while (loader.hasNext()) {
		if (!loader.loadNext()) {
			assets.reset();
			Window::terminate();
			throw initialize_error("could not to initialize assets");
		}
	}

	Audio::initialize();
	gui = new GUI();
    if (settings.ui.language == "auto") {
        settings.ui.language = platform::detect_locale();
    }
    setLanguage(settings.ui.language);
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
	setScreen(shared_ptr<Screen>(new MenuScreen(this)));
	
	std::cout << "-- preparing systems" << std::endl;

	Batch2D batch(1024);
	lastTime = Window::time();

	while (!Window::isShouldClose()){
		assert(screen != nullptr);
		updateTimers();
		updateHotkeys();

		gui->act(delta);
		screen->update(delta);
		screen->draw(delta);
		gui->draw(&batch, assets.get());

		Window::swapInterval(settings.display.swapInterval);
		Window::swapBuffers();
		Events::pollEvents();
	}
}

Engine::~Engine() {
	screen = nullptr;
	delete gui;

	Audio::finalize();

	std::cout << "-- shutting down" << std::endl;
    assets.reset();
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
	return assets.get();
}

void Engine::setScreen(shared_ptr<Screen> screen) {
	this->screen = screen;
}

const Content* Engine::getContent() const {
	return content.get();
}

vector<ContentPack>& Engine::getContentPacks() {
    return contentPacks;
}

EnginePaths* Engine::getPaths() {
	return paths;
}

void Engine::setLanguage(string locale) {
	settings.ui.language = locale;
	langs::setup(paths->getResources(), locale, contentPacks);
	menus::create_menus(this, gui->getMenu());
}

void Engine::loadContent() {
    auto resdir = paths->getResources();
    ContentBuilder contentBuilder;
    setup_definitions(&contentBuilder);
    
    vector<path> resRoots;
    for (auto& pack : contentPacks) {
        ContentLoader loader(&pack);
        loader.load(&contentBuilder);
        resRoots.push_back(pack.folder);
    }
    content.reset(contentBuilder.build());
    resPaths.reset(new ResPaths(resdir, resRoots));

    Shader::preprocessor->setPaths(resPaths.get());

    unique_ptr<Assets> new_assets(new Assets());
	std::cout << "-- loading assets" << std::endl;
	AssetsLoader loader(new_assets.get(), resPaths.get());
    AssetsLoader::createDefaults(loader);
    AssetsLoader::addDefaults(loader);
	while (loader.hasNext()) {
		if (!loader.loadNext()) {
			new_assets.reset();
			throw std::runtime_error("could not to load assets");
		}
	}
    assets->extend(*new_assets.get());
}
