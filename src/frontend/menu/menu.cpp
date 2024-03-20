#include "menu.h"

#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <glm/glm.hpp>

#include "../../graphics/ui/GUI.h"
#include "../../graphics/ui/gui_util.h"
#include "../../graphics/ui/elements/containers.h"
#include "../../graphics/ui/elements/controls.h"
#include "../screens.h"
#include "../UiDocument.h"

#include "../../coders/png.h"
#include "../../util/stringutil.h"
#include "../../files/engine_paths.h"
#include "../../files/WorldConverter.h"
#include "../../files/WorldFiles.h"
#include "../../world/World.h"
#include "../../world/WorldGenerators.h"
#include "../../world/Level.h"
#include "../../window/Events.h"
#include "../../window/Window.h"
#include "../../engine.h"
#include "../../settings.h"
#include "../../delegates.h"
#include "../../content/Content.h"
#include "../../content/ContentLUT.h"
#include "../../content/ContentPack.h"
#include "../locale/langs.h"

#include "menu_commons.h"

namespace fs = std::filesystem;
using namespace gui;

namespace menus {
    extern std::string generatorID;
}

static void load_page(Engine* engine, const std::string& name) {
    auto menu = engine->getGUI()->getMenu();
    auto file = engine->getResPaths()->find("layouts/pages/"+name+".xml");
    auto node = UiDocument::readElement(file);
    menu->addPage(name, node);
}

void menus::create_version_label(Engine* engine) {
    auto gui = engine->getGUI();
    auto vlabel = std::make_shared<gui::Label>(
        util::str2wstr_utf8(ENGINE_VERSION_STRING+" development build ")
    );
    vlabel->setZIndex(1000);
    vlabel->setColor(glm::vec4(1, 1, 1, 0.5f));
    vlabel->setPositionFunc([=]() {
        return glm::vec2(Window::width-vlabel->getSize().x, 2);
    });
    gui->add(vlabel);
}

static void show_content_missing(
    Engine* engine, 
    const Content* content, 
    std::shared_ptr<ContentLUT> lut
) {
    auto* gui = engine->getGUI();
    auto menu = gui->getMenu();
    auto panel = menus::create_page(engine, "missing-content", 500, 0.5f, 8);

    panel->add(std::make_shared<Label>(langs::get(L"menu.missing-content")));

    auto subpanel = std::make_shared<Panel>(glm::vec2(500, 100));
    subpanel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    subpanel->setScrollable(true);
    subpanel->setMaxLength(400);
    panel->add(subpanel);

    for (auto& entry : lut->getMissingContent()) {
        auto hpanel = std::make_shared<Panel>(glm::vec2(500, 30));
        hpanel->setColor(glm::vec4(0.0f));
        hpanel->setOrientation(Orientation::horizontal);
        
        auto namelabel = std::make_shared<Label>(util::str2wstr_utf8(entry.name));
        namelabel->setColor(glm::vec4(1.0f, 0.2f, 0.2f, 0.5f));

        auto contentname = util::str2wstr_utf8(contenttype_name(entry.type));
        auto typelabel = std::make_shared<Label>(L"["+contentname+L"]");
        typelabel->setColor(glm::vec4(0.5f));
        hpanel->add(typelabel);
        hpanel->add(namelabel);
        subpanel->add(hpanel);
    }

    panel->add(std::make_shared<Button>(
        langs::get(L"Back to Main Menu", L"menu"), glm::vec4(8.0f), [=](GUI*){
            menu->back();
        }
    ));
    menu->setPage("missing-content");
}

void show_process_panel(Engine* engine, std::shared_ptr<WorldConverter> converter, runnable postRunnable) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = menus::create_page(engine, "process", 400, 0.5f, 1);

    panel->add(std::make_shared<Label>(langs::get(L"Converting world...")));

    auto label = std::make_shared<Label>(L"0%");
    panel->add(label);

    uint initialTasks = converter->getTotalTasks();

    panel->listenInterval(0.01f, [=]() {
        if (!converter->hasNext()) {
            converter->write();

            menu->reset();
            menu->setPage("main", false);
            engine->getGUI()->postRunnable([=]() {
                postRunnable();
            });
            return;
        }
        converter->convertNext();

        uint tasksDone = initialTasks-converter->getTotalTasks();
        float progress = tasksDone/static_cast<float>(initialTasks);
        label->setText(
            std::to_wstring(tasksDone)+
            L"/"+std::to_wstring(initialTasks)+L" ("+
            std::to_wstring(int(progress*100))+L"%)"
        );
    });

    menu->reset();
    menu->setPage("process", false);
}

void show_convert_request(
    Engine* engine, 
    const Content* content, 
    std::shared_ptr<ContentLUT> lut,
    fs::path folder,
    runnable postRunnable
) {
    guiutil::confirm(engine->getGUI(), langs::get(L"world.convert-request"), [=]() {
        show_process_panel(engine, std::make_shared<WorldConverter>(folder, content, lut), postRunnable);
    }, L"", langs::get(L"Cancel"));
}

void create_languages_panel(Engine* engine) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = menus::create_page(engine, "languages", 400, 0.5f, 1);
    panel->setScrollable(true);

    std::vector<std::string> locales;
    for (auto& entry : langs::locales_info) {
        locales.push_back(entry.first);
    }
    std::sort(locales.begin(), locales.end());
    for (std::string& name : locales) {
        auto& locale = langs::locales_info.at(name);
        std::string& fullName = locale.name;

        auto button = std::make_shared<Button>(
            util::str2wstr_utf8(fullName), 
            glm::vec4(10.f),
            [=](GUI*) {
                engine->setLanguage(name);
                menu->back();
            }
        );
        panel->add(button);
    }
    panel->add(guiutil::backButton(menu));
}

void menus::open_world(std::string name, Engine* engine, bool confirmConvert) {
    auto paths = engine->getPaths();
    auto folder = paths->getWorldsFolder()/fs::u8path(name);
    try {
        engine->loadWorldContent(folder);
    } catch (const contentpack_error& error) {
        // could not to find or read pack
        guiutil::alert(
            engine->getGUI(), langs::get(L"error.pack-not-found")+L": "+
            util::str2wstr_utf8(error.getPackId())
        );
        return;
    } catch (const std::runtime_error& error) {
        guiutil::alert(
            engine->getGUI(), langs::get(L"Content Error", L"menu")+L": "+
            util::str2wstr_utf8(error.what())
        );
        return;
    }

    auto& packs = engine->getContentPacks();
    auto* content = engine->getContent();
    auto& settings = engine->getSettings();

    std::shared_ptr<ContentLUT> lut (World::checkIndices(folder, content));
    if (lut) {
        if (lut->hasMissingContent()) {
            show_content_missing(engine, content, lut);
        } else {
            if (confirmConvert) {
                show_process_panel(engine, std::make_shared<WorldConverter>(folder, content, lut), [=](){
                    open_world(name, engine, false);
                });
            } else {
                show_convert_request(engine, content, lut, folder, [=](){
                    open_world(name, engine, false);
                });
            }
        }
    } else {
        try {
            Level* level = World::load(folder, settings, content, packs);
            level->world->wfile->createDirectories();
            engine->setScreen(std::make_shared<LevelScreen>(engine, level));
        } catch (const world_load_error& error) {
            guiutil::alert(
                engine->getGUI(), langs::get(L"Error")+L": "+
                util::str2wstr_utf8(error.what())
            );
            return;
        }
    }
}

std::shared_ptr<Panel> create_worlds_panel(Engine* engine) {
    auto panel = std::make_shared<Panel>(glm::vec2(390, 0), glm::vec4(5.0f));
    panel->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.07f));
    panel->setMaxLength(400);

    auto paths = engine->getPaths();

    for (auto folder : paths->scanForWorlds()) {
        auto name = folder.filename().u8string();
        auto namews = util::str2wstr_utf8(name);

        auto btn = std::make_shared<Container>(glm::vec2(390, 46));
        btn->setColor(glm::vec4(0.06f, 0.12f, 0.18f, 0.7f));
        btn->setHoverColor(glm::vec4(0.09f, 0.17f, 0.2f, 0.6f));
        btn->listenAction([=](GUI*) {
            menus::open_world(name, engine, false);
        });
        btn->add(std::make_shared<Label>(namews), glm::vec2(8, 8));

        auto delbtn = guiutil::create(
            "<button color='#00000000' hover-color='#FFFFFF2B' padding='2,2,2,2'>"
            "    <image src='gui/delete_icon' size='32,32' color='#FFFFFF80'/>"
            "</button>"
        );
        delbtn->listenAction([=](GUI* gui) {
            guiutil::confirm(gui, langs::get(L"delete-confirm", L"world")+
            L" ("+util::str2wstr_utf8(folder.u8string())+L")", [=]() {
                std::cout << "deleting " << folder.u8string() << std::endl;
                fs::remove_all(folder);
                menus::refresh_menus(engine);
            });
        });
        btn->add(delbtn, glm::vec2(330, 3));

        panel->add(btn);
    }
    return panel;
}

void create_main_menu_panel(Engine* engine) {
    auto menu = engine->getGUI()->getMenu();

    auto panel = menus::create_page(engine, "main", 400, 0.0f, 1);
    panel->add(guiutil::gotoButton(L"New World", "new-world", menu));
    panel->add(create_worlds_panel(engine));
    panel->add(guiutil::gotoButton(L"Settings", "settings", menu));

    panel->add(std::make_shared<Button>(
        langs::get(L"Quit", L"menu"), glm::vec4(10.f), [](GUI*) {
            Window::setShouldClose(true);
        }
    ));
}

void menus::create_menus(Engine* engine) {
    menus::generatorID = WorldGenerators::getDefaultGeneratorID();
    create_new_world_panel(engine);
    create_settings_panel(engine);
    create_languages_panel(engine);
    create_main_menu_panel(engine);
    create_world_generators_panel(engine);
    load_page(engine, "404");
}

void menus::refresh_menus(Engine* engine) {
    create_main_menu_panel(engine);
    create_new_world_panel(engine);
    create_world_generators_panel(engine);
    load_page(engine, "404");
}
