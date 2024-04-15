#include "menu.h"

#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <glm/glm.hpp>

#include "../../interfaces/Task.h"
#include "../../graphics/ui/GUI.h"
#include "../../graphics/ui/gui_util.h"
#include "../../graphics/ui/elements/containers.h"
#include "../../graphics/ui/elements/controls.h"
#include "../screens.h"
#include "../UiDocument.h"
#include "../../logic/scripting/scripting.h"

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

    auto subpanel = std::dynamic_pointer_cast<Panel>(guiutil::create(
        "<panel size='480,100' color='#00000080' scrollable='true' max-length='400'>"
        "</panel>"
    ));
    panel->add(subpanel);

    for (auto& entry : lut->getMissingContent()) {
        std::string contentname = contenttype_name(entry.type);
        auto hpanel = std::dynamic_pointer_cast<Panel>(guiutil::create(
            "<panel size='500,20' color='0' orientation='horizontal' padding='2'>"
                "<label color='#80808080'>["+contentname+"]</label>"
                "<label color='#FF333380'>"+entry.name+"</label>"
            "</panel>"
        ));
        subpanel->add(hpanel);
    }

    panel->add(std::make_shared<Button>(
        langs::get(L"Back to Main Menu", L"menu"), glm::vec4(8.0f), [=](GUI*){
            menu->back();
        }
    ));
    menu->setPage("missing-content");
}

void show_process_panel(Engine* engine, std::shared_ptr<Task> task, std::wstring text=L"") {
    auto menu = engine->getGUI()->getMenu();
    auto panel = menus::create_page(engine, "process", 400, 0.5f, 1);

    if (!text.empty()) {
        panel->add(std::make_shared<Label>(langs::get(text)));
    }

    auto label = std::make_shared<Label>(L"0%");
    panel->add(label);

    uint initialWork = task->getWorkTotal();

    panel->listenInterval(0.01f, [=]() {
        task->update();

        uint tasksDone = task->getWorkDone();
        float progress = tasksDone/static_cast<float>(initialWork);
        label->setText(
            std::to_wstring(tasksDone)+
            L"/"+std::to_wstring(initialWork)+L" ("+
            std::to_wstring(int(progress*100))+L"%)"
        );
    });

    menu->reset();
    menu->setPage("process", false);
}

std::shared_ptr<Task> create_converter(
    Engine* engine,
    fs::path folder, 
    const Content* content, 
    std::shared_ptr<ContentLUT> lut, 
    runnable postRunnable)
{
    return WorldConverter::startTask(folder, content, lut, [=](){
        auto menu = engine->getGUI()->getMenu();
        menu->reset();
        menu->setPage("main", false);
        engine->getGUI()->postRunnable([=]() {
            postRunnable();
        });
    }, true);
}

void show_convert_request(
    Engine* engine, 
    const Content* content, 
    std::shared_ptr<ContentLUT> lut,
    fs::path folder,
    runnable postRunnable
) {
    guiutil::confirm(engine->getGUI(), langs::get(L"world.convert-request"), [=]() {
        auto converter = create_converter(engine, folder, content, lut, postRunnable);
        show_process_panel(engine, converter, L"Converting world...");
    }, L"", langs::get(L"Cancel"));
}

void menus::open_world(std::string name, Engine* engine, bool confirmConvert) {
    auto paths = engine->getPaths();
    auto folder = paths->getWorldsFolder()/fs::u8path(name);
    try {
        engine->loadWorldContent(folder);
    } catch (const contentpack_error& error) {
        engine->setScreen(std::make_shared<MenuScreen>(engine));
        // could not to find or read pack
        guiutil::alert(
            engine->getGUI(), langs::get(L"error.pack-not-found")+L": "+
            util::str2wstr_utf8(error.getPackId())
        );
        return;
    } catch (const std::runtime_error& error) {
        engine->setScreen(std::make_shared<MenuScreen>(engine));
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
            engine->setScreen(std::make_shared<MenuScreen>(engine));
            show_content_missing(engine, content, lut);
        } else {
            if (confirmConvert) {
                show_process_panel(engine, create_converter(engine, folder, content, lut, [=]() {
                    open_world(name, engine, false);
                }), L"Converting world...");
            } else {
                show_convert_request(engine, content, lut, folder, [=](){
                    open_world(name, engine, false);
                });
            }
        }
    } else {
        try {
            Level* level = World::load(folder, settings, content, packs);
            level->getWorld()->wfile->createDirectories();
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

void menus::delete_world(std::string name, Engine* engine) {
    fs::path folder = engine->getPaths()->getWorldFolder(name);
    guiutil::confirm(engine->getGUI(), langs::get(L"delete-confirm", L"world")+
    L" ("+util::str2wstr_utf8(folder.u8string())+L")", [=]() {
        std::cout << "deleting " << folder.u8string() << std::endl;
        fs::remove_all(folder);
        menus::refresh_menus(engine);
    });
}

static void add_page_loader(Engine* engine, const std::string& name) {
    auto menu = engine->getGUI()->getMenu();
    auto file = engine->getResPaths()->find("layouts/pages/"+name+".xml");
    auto fullname = "core:pages/"+name;
    menu->addSupplier(name, [=]() {
        auto document = UiDocument::read(0, fullname, file).release();
        engine->getAssets()->store(document, fullname);
        scripting::on_ui_open(document, nullptr, glm::ivec3());
        return document->getRoot();
    });
}

void menus::create_menus(Engine* engine) {
    menus::generatorID = WorldGenerators::getDefaultGeneratorID();
    create_new_world_panel(engine);
    create_settings_panel(engine);
    add_page_loader(engine, "languages");
    create_world_generators_panel(engine);
    add_page_loader(engine, "main");
    add_page_loader(engine, "404");
}

void menus::refresh_menus(Engine* engine) {
    create_new_world_panel(engine);
    create_world_generators_panel(engine);
    add_page_loader(engine, "main");
    add_page_loader(engine, "404");
    add_page_loader(engine, "settings-audio");
}
