#include "engine.h"

#include <memory>
#include <iostream>
#include <assert.h>
#include <vector>
#include <glm/glm.hpp>
#include <filesystem>
#include <unordered_set>
#define GLEW_STATIC

#include "audio/Audio.h"
#include "assets/Assets.h"
#include "assets/AssetsLoader.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "window/input.h"
#include "graphics/Batch2D.h"
#include "graphics/GfxContext.h"
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
#include "logic/scripting/scripting.h"

#include "core_defs.h"

namespace fs = std::filesystem;

Engine::Engine(EngineSettings& settings, EnginePaths* paths) 
    : settings(settings), paths(paths) 
{    
	if (Window::initialize(settings.display)){
		throw initialize_error("could not initialize window");
	}

    auto resdir = paths->getResources();
    scripting::initialize(this);

	std::cout << "-- loading assets" << std::endl;
    std::vector<fs::path> roots {resdir};

    resPaths = std::make_unique<ResPaths>(resdir, roots);
    assets = std::make_unique<Assets>();


	AssetsLoader loader(assets.get(), resPaths.get());
	AssetsLoader::addDefaults(loader, nullptr);

    Shader::preprocessor->setPaths(resPaths.get());
	while (loader.hasNext()) {
		if (!loader.loadNext()) {
			assets.reset();
            scripting::close();
			Window::terminate();
			throw initialize_error("could not to load assets");
		}
	}

	Audio::initialize();
	gui = std::make_unique<gui::GUI>();
    if (settings.ui.language == "auto") {
        settings.ui.language = langs::locale_by_envlocale(platform::detect_locale(), paths->getResources());
    }
    setLanguage(settings.ui.language);
}

void Engine::updateTimers() {
	frame++;
	double currentTime = Window::time();
	delta = currentTime - lastTime;
	lastTime = currentTime;
}

void Engine::updateHotkeys() {
	if (Events::jpressed(keycode::F2)) {
		std::unique_ptr<ImageData> image(Window::takeScreenshot());
		image->flipY();
		fs::path filename = paths->getScreenshotFile("png");
		png::write_image(filename.string(), image.get());
		std::cout << "saved screenshot as " << filename << std::endl;
	}
	if (Events::jpressed(keycode::F11)) {
		Window::toggleFullscreen();
	}
}

void Engine::mainloop() {
    setScreen(std::make_shared<MenuScreen>(this));

	Batch2D batch(1024);
	lastTime = Window::time();

    std::cout << "-- initialized" << std::endl;
	while (!Window::isShouldClose()){
		assert(screen != nullptr);
		updateTimers();
		updateHotkeys();

		gui->act(delta);
		screen->update(delta);

        if (!Window::isIconified()) {
		    screen->draw(delta);

            Viewport viewport(Window::width, Window::height);
            GfxContext ctx(nullptr, viewport, &batch);
		    gui->draw(&ctx, assets.get());
            
		    Window::swapInterval(settings.display.swapInterval);
        } else {
            Window::swapInterval(1);
        }
        Window::swapBuffers();
		Events::pollEvents();
	}
}

Engine::~Engine() {
    std::cout << "-- shutting down" << std::endl;
	screen.reset();
	content.reset();

	Audio::finalize();
    assets.reset();
    scripting::close();
	Window::terminate();
	std::cout << "-- engine finished" << std::endl;
}

inline const std::string checkPacks(const std::unordered_set<std::string>& packs, const std::vector<std::string>& dependencies) {
    for (const std::string& str : dependencies) if (packs.find(str) == packs.end()) return str;
    return "";
}

void Engine::loadContent() {
    auto resdir = paths->getResources();
    ContentBuilder contentBuilder;
    corecontent::setup(&contentBuilder);
    paths->setContentPacks(&contentPacks);

    std::vector<fs::path> resRoots;
    std::vector<ContentPack> srcPacks = contentPacks;
    contentPacks.clear();

	std::string missingDependency;
	std::unordered_set<std::string> loadedPacks, existingPacks;
	for (const auto& item : srcPacks) { existingPacks.insert(item.id); }

	while(existingPacks.size() > loadedPacks.size()) {
		for (auto& pack : srcPacks) {
			if(loadedPacks.find(pack.id) != loadedPacks.end()) continue;
			missingDependency = checkPacks(existingPacks, pack.dependencies);
			if(!missingDependency.empty()) 
                throw contentpack_error(pack.id, pack.folder, "missing dependency '"+missingDependency+"'");
			if(pack.dependencies.empty() || checkPacks(loadedPacks, pack.dependencies).empty()) {
				loadedPacks.insert(pack.id);
				resRoots.push_back(pack.folder);
				contentPacks.push_back(pack);
				ContentLoader loader(&pack);
				loader.load(contentBuilder);
			}
		}
    }
    
    content.reset(contentBuilder.build());
    resPaths.reset(new ResPaths(resdir, resRoots));

    Shader::preprocessor->setPaths(resPaths.get());

    std::unique_ptr<Assets> new_assets(new Assets());
	std::cout << "-- loading assets" << std::endl;
	AssetsLoader loader(new_assets.get(), resPaths.get());
    AssetsLoader::addDefaults(loader, content.get());
	while (loader.hasNext()) {
		if (!loader.loadNext()) {
			new_assets.reset();
			throw std::runtime_error("could not to load assets");
		}
	}
    assets->extend(*new_assets.get());
}

void Engine::loadWorldContent(const fs::path& folder) {
    contentPacks.clear();
    auto packNames = ContentPack::worldPacksList(folder);
    ContentPack::readPacks(paths, contentPacks, packNames, folder);
    loadContent();
}

void Engine::loadAllPacks() {
	auto resdir = paths->getResources();
	contentPacks.clear();
	ContentPack::scan(paths, contentPacks);
}

void Engine::setScreen(std::shared_ptr<Screen> screen) {
	this->screen = screen;
}

void Engine::setLanguage(std::string locale) {
	settings.ui.language = locale;
	langs::setup(paths->getResources(), locale, contentPacks);
	menus::create_menus(this);
}

gui::GUI* Engine::getGUI() {
	return gui.get();
}

EngineSettings& Engine::getSettings() {
	return settings;
}

Assets* Engine::getAssets() {
	return assets.get();
}

const Content* Engine::getContent() const {
	return content.get();
}

std::vector<ContentPack>& Engine::getContentPacks() {
    return contentPacks;
}

EnginePaths* Engine::getPaths() {
	return paths;
}

std::shared_ptr<Screen> Engine::getScreen() {
    return screen;
}
