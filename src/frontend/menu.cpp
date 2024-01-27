#include "menu.h"

#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <glm/glm.hpp>

#include "gui/GUI.h"
#include "gui/panels.h"
#include "gui/controls.h"
#include "screens.h"

#include "../coders/png.h"
#include "../util/stringutil.h"
#include "../files/engine_paths.h"
#include "../files/WorldConverter.h"
#include "../files/WorldFiles.h"
#include "../world/World.h"
#include "../world/Level.h"
#include "../window/Events.h"
#include "../window/Window.h"
#include "../engine.h"
#include "../settings.h"
#include "../content/Content.h"
#include "../content/ContentLUT.h"
#include "../content/ContentPack.h"

#include "gui/gui_util.h"
#include "locale/langs.h"

using glm::vec2;
using glm::vec4;

namespace fs = std::filesystem;
using namespace gui;

inline uint64_t randU64() {
    srand(time(NULL));
    return rand() ^ (rand() << 8) ^ 
        (rand() << 16) ^ (rand() << 24) ^
        ((uint64_t)rand() << 32) ^ 
        ((uint64_t)rand() << 40) ^
        ((uint64_t)rand() << 56);
}

std::shared_ptr<Panel> create_page(
        Engine* engine, 
        std::string name, 
        int width, 
        float opacity, 
        int interval) {
    PagesControl* menu = engine->getGUI()->getMenu();
    Panel* panel = new Panel(vec2(width, 200), vec4(8.0f), interval);
    panel->color(vec4(0.0f, 0.0f, 0.0f, opacity));

    std::shared_ptr<Panel> ptr (panel);
    menu->add(name, ptr);
    return ptr;
}

Button* create_button(std::wstring text, 
                      glm::vec4 padding, 
                      glm::vec4 margin, 
                      gui::onaction action) {

    auto btn = new Button(langs::get(text, L"menu"), 
                          padding, margin);
    btn->listenAction(action);
    return btn;
}

void show_content_missing(Engine* engine, const Content* content, 
                          std::shared_ptr<ContentLUT> lut) {
    auto* gui = engine->getGUI();
    auto* menu = gui->getMenu();
    auto panel = create_page(engine, "missing-content", 500, 0.5f, 8);

    panel->add(new Label(langs::get(L"menu.missing-content")));

    Panel* subpanel = new Panel(vec2(500, 100));
    subpanel->color(vec4(0.0f, 0.0f, 0.0f, 0.5f));

    for (auto& entry : lut->getMissingContent()) {
         Panel* hpanel = new Panel(vec2(500, 30));
        hpanel->color(vec4(0.0f));
        hpanel->orientation(Orientation::horizontal);
        
        Label* namelabel = new Label(util::str2wstr_utf8(entry.name));
        namelabel->color(vec4(1.0f, 0.2f, 0.2f, 0.5f));

        auto contentname = util::str2wstr_utf8(contenttype_name(entry.type));
        Label* typelabel = new Label(L"["+contentname+L"]");
        typelabel->color(vec4(0.5f));
        hpanel->add(typelabel);
        hpanel->add(namelabel);
        subpanel->add(hpanel);
    }
    subpanel->maxLength(400);
    panel->add(subpanel);

    panel->add((new Button(langs::get(L"Back to Main Menu", L"menu"), 
                           vec4(8.0f)))
    ->listenAction([=](GUI*){
        menu->back();
    }));
    menu->set("missing-content");
}

void show_convert_request(
        Engine* engine, 
        const Content* content, 
        std::shared_ptr<ContentLUT> lut,
        fs::path folder) {
    guiutil::confirm(engine->getGUI(), langs::get(L"world.convert-request"),
    [=]() {
        // TODO: add multithreading here
        auto converter = std::make_unique<WorldConverter>(folder, content, lut);
        while (converter->hasNext()) {
            converter->convertNext();
        }
        converter->write();
    }, L"", langs::get(L"Cancel"));
}

void create_languages_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "languages", 400, 0.5f, 1);
    panel->scrollable(true);

    std::vector<std::string> locales;
    for (auto& entry : langs::locales_info) {
        locales.push_back(entry.first);
    }
    std::sort(locales.begin(), locales.end());
    for (std::string& name : locales) {
        auto& locale = langs::locales_info.at(name);
        std::string& fullName = locale.name;

        Button* button = new Button(util::str2wstr_utf8(fullName), vec4(10.f));
        button->listenAction([=](GUI*) {
            engine->setLanguage(name);
            menu->back();
        });
        panel->add(button);
    }
    panel->add(guiutil::backButton(menu));
}

void open_world(std::string name, Engine* engine) {
    auto paths = engine->getPaths();
    auto folder = paths->getWorldsFolder()/fs::u8path(name);
    try {
        engine->loadWorldContent(folder);
    } catch (const contentpack_error& error) {
        // could not to find or read pack
        guiutil::alert(engine->getGUI(), 
                       langs::get(L"error.pack-not-found")+
                       L": "+util::str2wstr_utf8(error.getPackId()));
        return;
    } catch (const std::runtime_error& error) {
        guiutil::alert(engine->getGUI(),
                       langs::get(L"Content Error", L"menu")+
                       L": "+util::str2wstr_utf8(error.what()));
        return;
    }
    paths->setWorldFolder(folder);

    auto& packs = engine->getContentPacks();
    auto* content = engine->getContent();
    auto& settings = engine->getSettings();
    fs::create_directories(folder);
    std::shared_ptr<ContentLUT> lut (World::checkIndices(folder, content));
    if (lut) {
        if (lut->hasMissingContent()) {
            show_content_missing(engine, content, lut);
        } else {
            show_convert_request(engine, content, lut, folder);
        }
    } else {
        try {
            Level* level = World::load(folder, settings, content, packs);
            engine->setScreen(std::make_shared<LevelScreen>(engine, level));
        } catch (const world_load_error& error) {
            guiutil::alert(engine->getGUI(), 
                        langs::get(L"Error")+
                        L": "+util::str2wstr_utf8(error.what()));
            return;
        }
    }
}

Panel* create_worlds_panel(Engine* engine) {
    auto panel = new Panel(vec2(390, 200), vec4(5.0f));
    panel->color(vec4(1.0f, 1.0f, 1.0f, 0.07f));
    panel->maxLength(400);

    auto paths = engine->getPaths();

    for (auto folder : paths->scanForWorlds()) {
        auto name = folder.filename().u8string();
        auto namews = util::str2wstr_utf8(name);

        auto btn = std::make_shared<RichButton>(vec2(390, 46));
        btn->color(vec4(1.0f, 1.0f, 1.0f, 0.1f));
        btn->setHoverColor(vec4(1.0f, 1.0f, 1.0f, 0.17f));

        auto label = std::make_shared<Label>(namews);
        label->setInteractive(false);
        btn->add(label, vec2(8, 8));
        btn->listenAction([=](GUI*) {
            open_world(name, engine);
        });

        auto image = std::make_shared<Image>("gui/delete_icon", vec2(32, 32));
        image->color(vec4(1, 1, 1, 0.5f));

        auto delbtn = std::make_shared<Button>(image, vec4(2));
        delbtn->color(vec4(0.0f));
        delbtn->setHoverColor(vec4(1.0f, 1.0f, 1.0f, 0.17f));
        
        btn->add(delbtn, vec2(330, 3));

        delbtn->listenAction([=](GUI* gui) {
            guiutil::confirm(gui, langs::get(L"delete-confirm", L"world")+
            L" ("+util::str2wstr_utf8(folder.u8string())+L")", [=]() 
            {
                std::cout << "deleting " << folder.u8string() << std::endl;
                fs::remove_all(folder);
                menus::refresh_menus(engine, gui->getMenu());
            });
        });

        panel->add(btn);
    }
    return panel;
}

void create_main_menu_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "main", 400, 0.0f, 1);
    panel->add(guiutil::gotoButton(L"New World", "new-world", menu));
    panel->add(create_worlds_panel(engine));
    panel->add(guiutil::gotoButton(L"Settings", "settings", menu));
    panel->add((new Button(langs::get(L"Quit", L"menu"), vec4(10.f)))
    ->listenAction([](GUI* gui) {
        Window::setShouldClose(true);
    }));
}

typedef std::function<void(const ContentPack& pack)> packconsumer;

std::shared_ptr<Panel> create_packs_panel(const std::vector<ContentPack>& packs, Engine* engine, bool backbutton, packconsumer callback) {
    auto assets = engine->getAssets();
    auto panel = std::make_shared<Panel>(vec2(400, 200), vec4(5.0f));
    panel->color(vec4(1.0f, 1.0f, 1.0f, 0.07f));
    panel->maxLength(400);
    panel->scrollable(true);

    for (auto& pack : packs) {
        auto packpanel = std::make_shared<RichButton>(vec2(390, 80));
        if (callback) {
            packpanel->listenAction([=](GUI*) {
                callback(pack);
            });
        }
        auto idlabel = std::make_shared<Label>("["+pack.id+"]");
        idlabel->color(vec4(1, 1, 1, 0.5f));
        packpanel->add(idlabel, vec2(360-idlabel->size().x, 2));

        auto titlelabel = std::make_shared<Label>(pack.title);
        packpanel->add(titlelabel, vec2(78, 6));

        std::string icon = pack.id+".icon";
        if (assets->getTexture(icon) == nullptr) {
            auto iconfile = pack.folder/fs::path("icon.png");
            if (fs::is_regular_file(iconfile)) {
                assets->store(png::load_texture(iconfile.string()), icon);
            } else {
                icon = "gui/no_icon";
            }
        }

        if (!pack.creator.empty()) {
            auto creatorlabel = std::make_shared<Label>("@"+pack.creator);
            creatorlabel->color(vec4(0.8f, 1.0f, 0.9f, 0.7f));
            packpanel->add(creatorlabel, vec2(360-creatorlabel->size().x, 60));
        }

        auto descriptionlabel = std::make_shared<Label>(pack.description);
        descriptionlabel->color(vec4(1, 1, 1, 0.7f));
        packpanel->add(descriptionlabel, vec2(80, 28));

        packpanel->add(std::make_shared<Image>(icon, glm::vec2(64)), vec2(8));

        packpanel->color(vec4(0.06f, 0.12f, 0.18f, 0.7f));
        panel->add(packpanel);
    }
    if (backbutton)
        panel->add(guiutil::backButton(engine->getGUI()->getMenu()));
    return panel;
}

// TODO: refactor
void create_content_panel(Engine* engine, PagesControl* menu) {
    auto paths = engine->getPaths();
    auto mainPanel = create_page(engine, "content", 400, 0.0f, 5);

    std::vector<ContentPack> scanned;
    ContentPack::scan(engine->getPaths(), scanned);
    for (const auto& pack : engine->getContentPacks()) {
        for (size_t i = 0; i < scanned.size(); i++) {
            if (scanned[i].id == pack.id) {
                scanned.erase(scanned.begin()+i);
                i--;
            }
        }
    }

    auto panel = create_packs_panel(engine->getContentPacks(), engine, false, nullptr);
    mainPanel->add(panel);
    mainPanel->add(create_button(
    langs::get(L"Add", L"content"), vec4(10.0f), vec4(1), [=](GUI* gui) {
        auto panel = create_packs_panel(scanned, engine, true, 
        [=](const ContentPack& pack) {
            auto screen = dynamic_cast<LevelScreen*>(engine->getScreen().get());
            auto level = screen->getLevel();
            auto world = level->getWorld();

            auto worldFolder = paths->getWorldFolder();
            for (const auto& dependency : pack.dependencies) {
                fs::path folder = ContentPack::findPack(paths, worldFolder, dependency);
                if (!fs::is_directory(folder)) {
                    guiutil::alert(gui, langs::get(L"error.dependency-not-found")+
                                   L": "+util::str2wstr_utf8(dependency));
                    return;
                }
                if (!world->hasPack(dependency)) {
                    world->wfile->addPack(dependency);
                }
            }
            
            world->wfile->addPack(pack.id);

            std::string wname = world->getName();
            engine->setScreen(nullptr);
            engine->setScreen(std::make_shared<MenuScreen>(engine));
            open_world(wname, engine);
        });
        menu->add("content-packs", panel);
        menu->set("content-packs");
    }));
    mainPanel->add(guiutil::backButton(menu));
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

void create_new_world_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "new-world", 400, 0.0f, 1);

    panel->add(std::make_shared<Label>(langs::get(L"Name", L"world")));
    auto nameInput = std::make_shared<TextBox>(L"New World", vec4(6.0f));
    nameInput->textValidator([=](const std::wstring& text) {
        EnginePaths* paths = engine->getPaths();
        std::string textutf8 = util::wstr2str_utf8(text);
        return util::is_valid_filename(text) && 
                !paths->isWorldNameUsed(textutf8);
    });
    panel->add(nameInput);

    panel->add(std::make_shared<Label>(langs::get(L"Seed", L"world")));
    auto seedstr = std::to_wstring(randU64());
    auto seedInput = std::make_shared<TextBox>(seedstr, vec4(6.0f));
    panel->add(seedInput);

    panel->add(create_button( L"Create World", vec4(10), vec4(1, 20, 1, 1), 
    [=](GUI*) {
        if (!nameInput->validate())
            return;

        std::wstring name = nameInput->text();
        std::string nameutf8 = util::wstr2str_utf8(name);
        EnginePaths* paths = engine->getPaths();

        std::wstring seedstr = seedInput->text();
        uint64_t seed = str2seed(seedstr);
        std::cout << "world seed: " << seed << std::endl;

        auto folder = paths->getWorldsFolder()/fs::u8path(nameutf8);

        try {
            engine->loadAllPacks();
            engine->loadContent();
            paths->setWorldFolder(folder);
        } catch (const contentpack_error& error) {
            guiutil::alert(engine->getGUI(),
                        langs::get(L"Content Error", L"menu")+
                        L":\n"+util::str2wstr_utf8(std::string(error.what())+
                                "\npack '"+error.getPackId()+"' from "+
                                error.getFolder().u8string()));
            return;
        } catch (const std::runtime_error& error) {
            guiutil::alert(engine->getGUI(),
                        langs::get(L"Content Error", L"menu")+
                        L": "+util::str2wstr_utf8(error.what()));
            return;
        }

        Level* level = World::create(
            nameutf8, folder, seed, 
            engine->getSettings(), 
            engine->getContent(),
            engine->getContentPacks());
        engine->setScreen(std::make_shared<LevelScreen>(engine, level));
    }));
    panel->add(guiutil::backButton(menu));
}

void create_controls_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "controls", 400, 0.0f, 1);

    /* Camera sensitivity setting track bar */{
        panel->add((new Label(L""))->textSupplier([=]() {
            float s = engine->getSettings().camera.sensitivity;
            return langs::get(L"Mouse Sensitivity", L"settings")+L": "+
                   util::to_wstring(s, 1);
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
        std::string bindname = entry.first;
        
        Panel* subpanel = new Panel(vec2(400, 40), vec4(5.0f), 1.0f);
        subpanel->color(vec4(0.0f));
        subpanel->orientation(Orientation::horizontal);

        InputBindBox* bindbox = new InputBindBox(entry.second);
        subpanel->add(bindbox);
        Label* label = new Label(langs::get(util::str2wstr_utf8(bindname)));
        label->margin(vec4(6.0f));
        subpanel->add(label);
        scrollPanel->add(subpanel);
    }
    panel->add(scrollPanel);
    panel->add(guiutil::backButton(menu));
}

void create_settings_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "settings", 400, 0.0f, 1);

    /* Load Distance setting track bar */{
        panel->add((new Label(L""))->textSupplier([=]() {
            return langs::get(L"Load Distance", L"settings")+L": " + 
                std::to_wstring(engine->getSettings().chunks.loadDistance);
        }));

        TrackBar* trackbar = new TrackBar(3, 66, 10, 1, 3);
        trackbar->supplier([=]() {
            return engine->getSettings().chunks.loadDistance;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().chunks.loadDistance = static_cast<uint>(value);
        });
        panel->add(trackbar);
    }

    /* Load Speed setting track bar */{
        panel->add((new Label(L""))->textSupplier([=]() {
            return langs::get(L"Load Speed", L"settings")+L": " + 
                std::to_wstring(engine->getSettings().chunks.loadSpeed);
        }));

        TrackBar* trackbar = new TrackBar(1, 32, 10, 1, 1);
        trackbar->supplier([=]() {
            return engine->getSettings().chunks.loadSpeed;
        });
        trackbar->consumer([=](double value) {
            engine->getSettings().chunks.loadSpeed = static_cast<uint>(value);
        });
        panel->add(trackbar);
    }

    /* Fog Curve setting track bar */{
        panel->add((new Label(L""))->textSupplier([=]() {
            float value = engine->getSettings().graphics.fogCurve;
            return langs::get(L"Fog Curve", L"settings")+L": " +
                   util::to_wstring(value, 1);
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
            return langs::get(L"FOV", L"settings")+L": "+std::to_wstring(fov)+L"°";
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
        auto checkbox = new FullCheckBox(langs::get(L"V-Sync", L"settings"), vec2(400, 32));
        checkbox->supplier([=]() {
            return engine->getSettings().display.swapInterval != 0;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().display.swapInterval = checked;
        });
        panel->add(checkbox);
    }

    /* Backlight checkbox */{
        auto checkbox = new FullCheckBox(langs::get(L"Backlight", L"settings"), vec2(400, 32));
        checkbox->supplier([=]() {
            return engine->getSettings().graphics.backlight != 0;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().graphics.backlight = checked;
        });
        panel->add(checkbox);
    }

    std::string langName = langs::locales_info.at(langs::current->getId()).name;
    panel->add(guiutil::gotoButton(
        langs::get(L"Language", L"settings")+L": "+
        util::str2wstr_utf8(langName), 
        "languages", menu));

    panel->add(guiutil::gotoButton(L"Controls", "controls", menu));
    panel->add(guiutil::backButton(menu));
}

void create_pause_panel(Engine* engine, PagesControl* menu) {
    auto panel = create_page(engine, "pause", 400, 0.0f, 1);

    panel->add(create_button(L"Continue", vec4(10.0f), vec4(1), [=](GUI*){
        menu->reset();
    }));
    panel->add(create_button(L"Content", vec4(10.0f), vec4(1), [=](GUI*) {
        create_content_panel(engine, menu);
        menu->set("content");
    }));
    panel->add(guiutil::gotoButton(L"Settings", "settings", menu));

    panel->add(create_button(L"Save and Quit to Menu", vec4(10.f), vec4(1), [=](GUI*){
        // save world and destroy LevelScreen
        engine->setScreen(nullptr);
        // create and go to menu screen
        engine->setScreen(std::make_shared<MenuScreen>(engine));
    }));
}

void menus::create_menus(Engine* engine, PagesControl* menu) {
    create_new_world_panel(engine, menu);
    create_settings_panel(engine, menu);
    create_controls_panel(engine, menu);
    create_pause_panel(engine, menu);
    create_languages_panel(engine, menu);
    create_main_menu_panel(engine, menu);
}

void menus::refresh_menus(Engine* engine, PagesControl* menu) {
    create_main_menu_panel(engine, menu);
    create_new_world_panel(engine, menu);
}
