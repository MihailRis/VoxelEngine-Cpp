#include "screens.h"

#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <filesystem>

#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../graphics/Shader.h"
#include "../assets/Assets.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../objects/Player.h"
#include "../voxels/ChunksController.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "world_render.h"
#include "hud.h"
#include "gui/GUI.h"
#include "gui/panels.h"
#include "gui/controls.h"
#include "../engine.h"
#include "../files/engine_files.h"
#include "../util/stringutil.h"

using std::wstring;
using glm::vec3;
using glm::vec4;
using std::shared_ptr;
using std::filesystem::path;
using std::filesystem::directory_iterator;
using namespace gui;

MenuScreen::MenuScreen(Engine* engine_) : Screen(engine_) {
    Panel* panel = new Panel(vec2(400, 200), vec4(5.0f), 1.0f);
    panel->color(vec4(0.0f));
	panel->setCoord(vec2(10, 10));

    {
        auto button = new Button(L"Continue", vec4(12.0f, 10.0f, 12.0f, 10.0f));
        button->listenAction([this, panel](GUI*) {
            std::cout << "-- loading world" << std::endl;
            EngineSettings& settings = engine->getSettings();
            path folder = enginefs::get_worlds_folder()/path("world");
            World* world = new World("world", folder, 42, settings);

            auto screen = new LevelScreen(engine, world->loadLevel(settings));
            engine->setScreen(shared_ptr<Screen>(screen));
        });
        panel->add(shared_ptr<UINode>(button));
    }
    // ATTENTION: FUNCTIONALITY INCOMPLETE ZONE
    /*Panel* worldsPanel = new Panel(vec2(390, 200), vec4(5.0f));
    worldsPanel->color(vec4(0.1f));
    for (auto const& entry : directory_iterator(enginefs::get_worlds_folder())) {
        std::string name = entry.path().filename();
        Button* button = new Button(util::str2wstr_utf8(name), vec4(10.0f, 8.0f, 10.0f, 8.0f));
        button->color(vec4(0.5f));
        button->listenAction([this, panel, name](GUI*) {
            EngineSettings& settings = engine->getSettings();
            World* world = new World(name, enginefs::get_worlds_folder()/name, 42, settings);
            engine->setScreen(new LevelScreen(engine, world->loadLevel(settings)));
        });
        worldsPanel->add(shared_ptr<UINode>(button));
    }
    panel->add(shared_ptr<UINode>(worldsPanel));*/
    
    {
        Button* button = new Button(L"Quit", vec4(12.0f, 10.0f, 12.0f, 10.0f));
        button->listenAction([this](GUI*) {
            Window::setShouldClose(true);
        });
        panel->add(shared_ptr<UINode>(button));
    }
    this->panel = shared_ptr<UINode>(panel);
    engine->getGUI()->add(this->panel);
}

MenuScreen::~MenuScreen() {
    engine->getGUI()->remove(panel);
}

void MenuScreen::update(float delta) {
}

void MenuScreen::draw(float delta) {
    panel->setCoord((Window::size() - panel->size()) / 2.0f);
    
    Window::clear();
    Window::setBgColor(vec3(0.2f, 0.2f, 0.2f));
}

LevelScreen::LevelScreen(Engine* engine, Level* level) 
    : Screen(engine), 
      level(level) {
    worldRenderer = new WorldRenderer(level, engine->getAssets());
    hud = new HudRenderer(engine, level);
}

LevelScreen::~LevelScreen() {
    delete hud;
    delete worldRenderer;

	std::cout << "-- writing world" << std::endl;
    World* world = level->world;
	world->write(level, !engine->getSettings().debug.generatorTestMode);

    delete level;
	delete world;
}

void LevelScreen::updateHotkeys() {
    if (Events::jpressed(keycode::O)) {
        occlusion = !occlusion;
    }
    if (Events::jpressed(keycode::F3)) {
        level->player->debug = !level->player->debug;
    }
    if (Events::jpressed(keycode::F5)) {
        for (uint i = 0; i < level->chunks->volume; i++) {
            shared_ptr<Chunk> chunk = level->chunks->chunks[i];
            if (chunk != nullptr && chunk->isReady()) {
                chunk->setModified(true);
            }
        }
    }
}

void LevelScreen::update(float delta) {
    gui::GUI* gui = engine->getGUI();
    EngineSettings& settings = engine->getSettings();

    bool inputLocked = hud->isPause() || 
                       hud->isInventoryOpen() || 
                       gui->isFocusCaught();
    if (!inputLocked) {
        updateHotkeys();
    }
    level->updatePlayer(delta, !inputLocked, hud->isPause(), !inputLocked);
    level->update();
    level->chunksController->update(settings.chunks.loadSpeed);
}

void LevelScreen::draw(float delta) {
    EngineSettings& settings = engine->getSettings();
    Camera* camera = level->player->camera;

    float fogFactor = 18.0f / (float)settings.chunks.loadDistance;
    worldRenderer->draw(camera, occlusion, fogFactor, settings.graphics.fogCurve);
    hud->draw();
    if (level->player->debug) {
        hud->drawDebug( 1 / delta, occlusion);
    }
}
