#include "menu.h"

#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <glm/glm.hpp>

#include "../gui/GUI.h"
#include "../gui/containers.h"
#include "../gui/controls.h"
#include "../screens.h"

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

#include "../gui/gui_util.h"
#include "../locale/langs.h"

#include "menu_commons.h"

namespace fs = std::filesystem;
using namespace gui;

namespace menus {
    std::string generatorID;
}

inline uint64_t randU64() {
    srand(time(NULL));
    return rand() ^ (rand() << 8) ^ 
        (rand() << 16) ^ (rand() << 24) ^
        ((uint64_t)rand() << 32) ^ 
        ((uint64_t)rand() << 40) ^
        ((uint64_t)rand() << 56);
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

static std::string translate_generator_id(std::string& id) {
    int delimiterPosition = id.find(":");
    std::string pack = id.substr(0, delimiterPosition);
    std::string generator = id.substr(delimiterPosition + 1);

    if(pack == "core") {
        return util::wstr2str_utf8(langs::get(util::str2wstr_utf8(generator), L"world.generators"));
    } else {
        return id;
    }
}

void create_world_generators_panel(Engine* engine) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = menus::create_page(engine, "world_generators", 400, 0.5f, 1);
    panel->setScrollable(true);

    std::vector<std::string> generatorsIDs = WorldGenerators::getGeneratorsIDs();
    std::sort(generatorsIDs.begin(), generatorsIDs.end());
    for (std::string& id : generatorsIDs) {
        const std::string& fullName = translate_generator_id(id);
        auto button = std::make_shared<RichButton>(glm::vec2(80, 30));

        auto idlabel = std::make_shared<Label>("["+id+"]");
        idlabel->setColor(glm::vec4(1, 1, 1, 0.5f));
        idlabel->setSize(glm::vec2(300, 25));
        idlabel->setAlign(Align::right);

        button->add(idlabel, glm::vec2(80, 4));
        button->add(std::make_shared<Label>(fullName), glm::vec2(0, 8));
        button->listenAction(
            [=](GUI*) {
                menus::generatorID = id;
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

        auto btn = std::make_shared<RichButton>(glm::vec2(390, 46));
        btn->setColor(glm::vec4(0.06f, 0.12f, 0.18f, 0.7f));
        btn->setHoverColor(glm::vec4(0.09f, 0.17f, 0.2f, 0.6f));
        btn->listenAction([=](GUI*) {
            menus::open_world(name, engine, false);
        });
        btn->add(std::make_shared<Label>(namews), glm::vec2(8, 8));

        auto image = std::make_shared<Image>("gui/delete_icon", glm::vec2(32, 32));
        image->setColor(glm::vec4(1, 1, 1, 0.5f));

        auto delbtn = std::make_shared<Button>(image, glm::vec4(2));
        delbtn->setColor(glm::vec4(0.0f));
        delbtn->setHoverColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.17f));
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

inline uint64_t str2seed(std::wstring seedstr) {
    if (util::is_integer(seedstr)) {
        try {
            return std::stoull(seedstr);
        } catch (const std::out_of_range& err) {
            std::hash<std::wstring> hash;
            return hash(seedstr);
        }
    } else {
        std::hash<std::wstring> hash;
        return hash(seedstr);
    }
}

void create_new_world_panel(Engine* engine) {
    auto panel = menus::create_page(engine, "new-world", 400, 0.0f, 1);

    panel->add(std::make_shared<Label>(langs::get(L"Name", L"world")));
    auto nameInput = std::make_shared<TextBox>(L"New World", glm::vec4(6.0f));
    nameInput->setTextValidator([=](const std::wstring& text) {
        EnginePaths* paths = engine->getPaths();
        std::string textutf8 = util::wstr2str_utf8(text);
        return util::is_valid_filename(text) && 
                !paths->isWorldNameUsed(textutf8);
    });
    panel->add(nameInput);

    panel->add(std::make_shared<Label>(langs::get(L"Seed", L"world")));
    auto seedstr = std::to_wstring(randU64());
    auto seedInput = std::make_shared<TextBox>(seedstr, glm::vec4(6.0f));
    panel->add(seedInput);
    
    panel->add(guiutil::gotoButton(langs::get(L"World generator", L"world"), "world_generators", engine->getGUI()->getMenu()));

    panel->add(menus::create_button(L"Content", glm::vec4(10), glm::vec4(1), [=](GUI* gui) {
        engine->loadAllPacks();
        auto panel = menus::create_packs_panel(engine->getContentPacks(), engine, false, nullptr, nullptr);
        auto menu = gui->getMenu();
        menu->addPage("content-packs", panel);
        menu->setPage("content-packs");
    }));

    panel->add(menus::create_button(L"Create World", glm::vec4(10), glm::vec4(1, 20, 1, 1), 
    [=](GUI*) {
        if (!nameInput->validate())
            return;

        std::string name = util::wstr2str_utf8(nameInput->getText());
        uint64_t seed = str2seed(seedInput->getText());
        std::cout << "world seed: " << seed << std::endl;

        EnginePaths* paths = engine->getPaths();
        auto folder = paths->getWorldsFolder()/fs::u8path(name);
        try {
            engine->loadAllPacks();
            engine->loadContent();
            paths->setWorldFolder(folder);
        } catch (const contentpack_error& error) {
            guiutil::alert(
                engine->getGUI(),
                langs::get(L"Content Error", L"menu")+L":\n"+
                util::str2wstr_utf8(
                    std::string(error.what())+
                    "\npack '"+error.getPackId()+"' from "+
                    error.getFolder().u8string()
                )
            );
            return;
        } catch (const std::runtime_error& error) {
            guiutil::alert(
                engine->getGUI(),
                langs::get(L"Content Error", L"menu")+
                L": "+util::str2wstr_utf8(error.what())
            );
            return;
        }

        Level* level = World::create(
            name, menus::generatorID, folder, seed, 
            engine->getSettings(), 
            engine->getContent(),
            engine->getContentPacks()
        );
        level->world->wfile->createDirectories();
        menus::generatorID = WorldGenerators::getDefaultGeneratorID();
        engine->setScreen(std::make_shared<LevelScreen>(engine, level));
    }));
    panel->add(guiutil::backButton(engine->getGUI()->getMenu()));
}

void create_controls_panel(Engine* engine) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = menus::create_page(engine, "controls", 400, 0.0f, 1);

    /* Camera sensitivity setting track bar */{
        panel->add(menus::create_label([=]() {
            float s = engine->getSettings().camera.sensitivity;
            return langs::get(L"Mouse Sensitivity", L"settings")+L": "+
                   util::to_wstring(s, 1);
        }));

        auto trackbar = std::make_shared<TrackBar>(0.1, 10.0, 2.0, 0.1, 4);
        trackbar->setSupplier([=]() {
            return engine->getSettings().camera.sensitivity;
        });
        trackbar->setConsumer([=](double value) {
            engine->getSettings().camera.sensitivity = value;
        });
        panel->add(trackbar);
    }

    auto scrollPanel = std::make_shared<Panel>(glm::vec2(400, 200), glm::vec4(2.0f), 1.0f);
    scrollPanel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.3f));
    scrollPanel->setMaxLength(400);
    for (auto& entry : Events::bindings){
        std::string bindname = entry.first;
        
        auto subpanel = std::make_shared<Panel>(glm::vec2(400, 40), glm::vec4(5.0f), 1.0f);
        subpanel->setColor(glm::vec4(0.0f));
        subpanel->setOrientation(Orientation::horizontal);
        subpanel->add(std::make_shared<InputBindBox>(entry.second));

        auto label = std::make_shared<Label>(langs::get(util::str2wstr_utf8(bindname)));
        label->setMargin(glm::vec4(6.0f));
        subpanel->add(label);
        scrollPanel->add(subpanel);
    }
    panel->add(scrollPanel);
    panel->add(guiutil::backButton(menu));
}

void menus::create_menus(Engine* engine) {
    menus::generatorID = WorldGenerators::getDefaultGeneratorID();
    create_new_world_panel(engine);
    create_settings_panel(engine);
    create_controls_panel(engine);
    create_languages_panel(engine);
    create_main_menu_panel(engine);
    create_world_generators_panel(engine);
}

void menus::refresh_menus(Engine* engine) {
    create_main_menu_panel(engine);
    create_new_world_panel(engine);
    create_world_generators_panel(engine);
}
