#include "menu.h"

#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <glm/glm.hpp>

#include "gui/GUI.h"
#include "gui/panels.h"
#include "gui/controls.h"
#include "screens.h"
#include "../util/stringutil.h"
#include "../files/engine_paths.h"
#include "../world/World.h"
#include "../window/Events.h"
#include "../window/Window.h"
#include "../engine.h"
#include "../settings.h"

#include "gui/gui_util.h"

using glm::vec2;
using glm::vec4;
using std::string;
using std::wstring;
using std::shared_ptr;
using std::filesystem::path;
using std::filesystem::u8path;
using std::filesystem::directory_iterator;
using namespace gui;

Panel* create_main_menu_panel(Engine* engine, PagesControl* menu) {
    EnginePaths* paths = engine->getPaths();

    Panel* panel = new Panel(vec2(400, 200), vec4(5.0f), 1.0f);
    panel->color(vec4(0.0f));

    panel->add(guiutil::gotoButton(L"New World", "new-world", menu));

    Panel* worldsPanel = new Panel(vec2(390, 200), vec4(5.0f));
    worldsPanel->color(vec4(0.1f));
    worldsPanel->maxLength(400);
    path worldsFolder = paths->getWorldsFolder();
    if (std::filesystem::is_directory(worldsFolder)) {
        for (auto const& entry : directory_iterator(worldsFolder)) {
            if (!entry.is_directory()) {
                continue;
            }
            string name = entry.path().filename().string();
            Button* button = new Button(util::str2wstr_utf8(name), 
                                        vec4(10.0f, 8.0f, 10.0f, 8.0f));
            button->color(vec4(0.5f));
            button->listenAction([=](GUI*) {
                EngineSettings& settings = engine->getSettings();

                auto folder = paths->getWorldsFolder()/u8path(name);
                World* world = new World(name, folder, 42, settings);
                auto screen = new LevelScreen(engine, 
                    world->load(settings, engine->getContent()));
                engine->setScreen(shared_ptr<Screen>(screen));
            });
            worldsPanel->add(button);
        }
    }
    panel->add(worldsPanel);
    panel->add(guiutil::gotoButton(L"Settings", "settings", menu));
    panel->add((new Button(L"Quit", vec4(10.f)))->listenAction([](GUI* gui) {
        Window::setShouldClose(true);
    }));
    panel->refresh();
    return panel;
}

Panel* create_new_world_panel(Engine* engine, PagesControl* menu) {
    Panel* panel = new Panel(vec2(400, 200), vec4(5.0f), 1.0f);
    panel->color(vec4(0.0f));

    TextBox* worldNameInput;
    {
        Label* label = new Label(L"World Name");
        panel->add(label);

        TextBox* input = new TextBox(L"New World", vec4(6.0f));
        panel->add(input);
        worldNameInput = input;
    }

    TextBox* seedInput;
    {
        Label* label = new Label(L"Seed");
        panel->add(shared_ptr<UINode>(label));

        uint64_t randseed = rand() ^ (rand() << 8) ^ 
                        (rand() << 16) ^ (rand() << 24) ^
                        ((uint64_t)rand() << 32) ^ 
                        ((uint64_t)rand() << 40) ^
                        ((uint64_t)rand() << 56);

        seedInput = new TextBox(std::to_wstring(randseed), vec4(6.0f));
        panel->add(seedInput);
    }

    {
        Button* button = new Button(L"Create World", vec4(10.0f));
        button->margin(vec4(1, 20, 1, 1));
        vec4 basecolor = worldNameInput->color();   
        button->listenAction([=](GUI*) {
            wstring name = worldNameInput->text();
            string nameutf8 = util::wstr2str_utf8(name);
            EnginePaths* paths = engine->getPaths();

            // Basic validation
            if (!util::is_valid_filename(name) || 
                 paths->isWorldNameUsed(nameutf8)) {
                // blink red two times
                panel->listenInterval(0.1f, [worldNameInput, basecolor]() {
                    static bool flag = true;
                    if (flag) {
                        worldNameInput->color(vec4(0.3f, 0.0f, 0.0f, 0.5f));
                    } else {
                        worldNameInput->color(basecolor);
                    }
                    flag = !flag;
                }, 4);
                return;
            }

            wstring seedstr = seedInput->text();
            uint64_t seed;
            if (util::is_integer(seedstr)) {
                try {
                    seed = std::stoull(seedstr);
                } catch (const std::out_of_range& err) {
                    std::hash<wstring> hash;
                    seed = hash(seedstr);
                }
            } else {
                std::hash<wstring> hash;
                seed = hash(seedstr);
            }
            std::cout << "world seed: " << seed << std::endl;
            
            EngineSettings& settings = engine->getSettings();

            auto folder = paths->getWorldsFolder()/u8path(nameutf8);
            std::filesystem::create_directories(folder);
            World* world = new World(nameutf8, folder, seed, settings);
            auto screen = new LevelScreen(engine, world->create(settings, engine->getContent()));
            engine->setScreen(shared_ptr<Screen>(screen));
        });
        panel->add(button);
    }

    panel->add(guiutil::backButton(menu));
    panel->refresh();
    return panel;
}

Panel* create_controls_panel(Engine* engine, PagesControl* menu) {
    Panel* panel = new Panel(vec2(400, 200), vec4(2.0f), 1.0f);
    panel->color(vec4(0.0f));

    /* Camera sensitivity setting track bar */{
        panel->add((new Label(L""))->textSupplier([=]() {
            std::wstringstream ss;
            ss << std::fixed << std::setprecision(1);
            ss << engine->getSettings().camera.sensitivity;
            return L"Mouse Sensitivity: "+ss.str();
        }));

        TrackBar* trackbar = new TrackBar(0.1, 10.0, 2.0, 0.1, 4);
        trackbar->supplier([=]() {
            return engine->getSettings().camera.sensitivity;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().camera.sensitivity = value;
        });
        panel->add(trackbar);
    }

    Panel* scrollPanel = new Panel(vec2(400, 200), vec4(2.0f), 1.0f);
    scrollPanel->color(vec4(0.0f, 0.0f, 0.0f, 0.3f));
    scrollPanel->maxLength(400);
    for (auto& entry : Events::bindings){
        string bindname = entry.first;
        
        Panel* subpanel = new Panel(vec2(400, 40), vec4(5.0f), 1.0f);
        subpanel->color(vec4(0.0f));
        subpanel->orientation(Orientation::horizontal);

        InputBindBox* bindbox = new InputBindBox(entry.second);
        subpanel->add(bindbox);
        Label* label = new Label(util::str2wstr_utf8(bindname));
        label->margin(vec4(6.0f));
        subpanel->add(label);
        scrollPanel->add(subpanel);
    }
    panel->add(scrollPanel);

    panel->add(guiutil::backButton(menu));
    panel->refresh();
    return panel;
}

Panel* create_settings_panel(Engine* engine, PagesControl* menu) {
    Panel* panel = new Panel(vec2(400, 200), vec4(5.0f), 1.0f);
    panel->color(vec4(0.0f));

    // TODO: simplify repeating code for trackbars
    /* Load Distance setting track bar */{
        panel->add((new Label(L""))->textSupplier([=]() {
            return L"Load Distance: " + 
                std::to_wstring(engine->getSettings().chunks.loadDistance);
        }));

        TrackBar* trackbar = new TrackBar(3, 66, 10, 1, 3);
        trackbar->supplier([=]() {
            return engine->getSettings().chunks.loadDistance;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().chunks.loadDistance = value;
        });
        panel->add(trackbar);
    }

    /* Load Speed setting track bar */{
        panel->add((new Label(L""))->textSupplier([=]() {
            return L"Load Speed: " + 
                std::to_wstring(engine->getSettings().chunks.loadSpeed);
        }));

        TrackBar* trackbar = new TrackBar(1, 32, 10, 1, 1);
        trackbar->supplier([=]() {
            return engine->getSettings().chunks.loadSpeed;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().chunks.loadSpeed = value;
        });
        panel->add(trackbar);
    }

    /* Fog Curve setting track bar */{
        panel->add((new Label(L""))->textSupplier([=]() {
            std::wstringstream ss;
            ss << std::fixed << std::setprecision(1);
            ss << engine->getSettings().graphics.fogCurve;
            return L"Fog Curve: " + ss.str();
        }));

        TrackBar* trackbar = new TrackBar(1.0, 6.0, 1.0, 0.1, 2);
        trackbar->supplier([=]() {
            return engine->getSettings().graphics.fogCurve;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().graphics.fogCurve = value;
        });
        panel->add(trackbar);
    }

    /* Fov setting track bar */{
        panel->add((new Label(L""))->textSupplier([=]() {
            int fov = (int)engine->getSettings().camera.fov;
            return L"FOV: "+std::to_wstring(fov)+L"°";
        }));

        TrackBar* trackbar = new TrackBar(30.0, 120.0, 90, 1, 4);
        trackbar->supplier([=]() {
            return engine->getSettings().camera.fov;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().camera.fov = value;
        });
        panel->add(trackbar);
    }

    /* V-Sync checkbox */{
        Panel* checkpanel = new Panel(vec2(400, 32), vec4(5.0f), 1.0f);
        checkpanel->color(vec4(0.0f));
        checkpanel->orientation(Orientation::horizontal);

        CheckBox* checkbox = new CheckBox();
        checkbox->margin(vec4(0.0f, 0.0f, 5.0f, 0.0f));
        checkbox->supplier([=]() {
            return engine->getSettings().display.swapInterval != 0;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().display.swapInterval = checked;
        });
        checkpanel->add(checkbox);
        checkpanel->add(new Label(L"V-Sync"));

        panel->add(checkpanel);
    }

    /* Backlight checkbox */{
        Panel* checkpanel = new Panel(vec2(400, 32), vec4(5.0f), 1.0f);
        checkpanel->color(vec4(0.0f));
        checkpanel->orientation(Orientation::horizontal);

        CheckBox* checkbox = new CheckBox();
        checkbox->margin(vec4(0.0f, 0.0f, 5.0f, 0.0f));
        checkbox->supplier([=]() {
            return engine->getSettings().graphics.backlight != 0;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().graphics.backlight = checked;
        });
        checkpanel->add(checkbox);
        checkpanel->add(new Label(L"Backlight"));

        panel->add(checkpanel);
    }

    panel->add(guiutil::gotoButton(L"Controls", "controls", menu));
    panel->add(guiutil::backButton(menu));
    panel->refresh();
    return panel;
}

Panel* create_pause_panel(Engine* engine, PagesControl* menu) {
    Panel* panel = new Panel(vec2(400, 200));
	panel->color(vec4(0.0f));
	{
		Button* button = new Button(L"Continue", vec4(10.0f));
		button->listenAction([=](GUI*){
			menu->reset();
		});
		panel->add(shared_ptr<UINode>(button));
	}
    panel->add(guiutil::gotoButton(L"Settings", "settings", menu));
	{
		Button* button = new Button(L"Save and Quit to Menu", vec4(10.f));
		button->listenAction([engine](GUI*){
			engine->setScreen(shared_ptr<Screen>(new MenuScreen(engine)));
		});
		panel->add(shared_ptr<UINode>(button));
	}
    return panel;
}
