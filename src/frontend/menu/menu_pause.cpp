#include "menu.h"
#include "menu_commons.h"

#include "../../coders/png.h"
#include "../../content/ContentLUT.h"
#include "../../engine.h"
#include "../../files/WorldFiles.h"
#include "../../graphics/ui/gui_util.h"
#include "../../logic/LevelController.h"
#include "../../util/stringutil.h"
#include "../../world/Level.h"
#include "../../world/World.h"
#include "../locale/langs.h"
#include "../screens.h"

#include <glm/glm.hpp>

using namespace gui;

std::shared_ptr<Container> create_pack_panel(
    const ContentPack& pack,
    Engine* engine,
    packconsumer callback,
    packconsumer remover
) {
    auto assets = engine->getAssets();
    auto packpanel = std::dynamic_pointer_cast<Container>(guiutil::create(
        "<container size='540,80' color='#0F1E2DB2'>"
        "</container>"
    ));
    if (callback) {
        packpanel->listenAction([=](GUI*) {
            callback(pack);
        });
    }
    auto runtime = engine->getContent() ? engine->getContent()->getPackRuntime(pack.id) : nullptr;
    auto idtext = (runtime && runtime->getStats().hasSavingContent()) 
        ? "*["+pack.id+"]" 
        :  "["+pack.id+"]";

    packpanel->add(guiutil::create(
        "<label pos='215,2' color='#FFFFFF80' size='300,25' align='right'>" +
        idtext +
        "</label>"
    ));

    auto titlelabel = std::make_shared<Label>(pack.title);
    packpanel->add(titlelabel, glm::vec2(78, 6));

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
        packpanel->add(guiutil::create(
            "<label color='#CCFFE5B2' size='300,20' align='right' pos='215,60'>"+
                pack.creator+
            "</label>"
        ));
    }

    packpanel->add(guiutil::create(
        "<label pos='80,28' color='#FFFFFFB2'>" +
            pack.description +
        "</label>"
    ));

    packpanel->add(std::make_shared<Image>(icon, glm::vec2(64)), glm::vec2(8));

    if (remover && pack.id != "base") {
        auto rembtn = guiutil::create(
            "<button color='#00000000' hover-color='#FFFFFF2B'>"
                "<image src='gui/cross' size='32,32'/>"
            "</button>"
        );
        rembtn->listenAction([=](GUI* gui) {
            remover(pack);
        });
        packpanel->add(rembtn, glm::vec2(470, 22));
    }
    return packpanel;
}

std::shared_ptr<Panel> menus::create_packs_panel(
    const std::vector<ContentPack>& packs, 
    Engine* engine, 
    bool backbutton, 
    packconsumer callback,
    packconsumer remover
){
    auto panel = std::make_shared<Panel>(glm::vec2(550, 200), glm::vec4(5.0f));
    panel->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.07f));
    panel->setMaxLength(400);
    panel->setScrollable(true);

    for (auto& pack : packs) {
        panel->add(create_pack_panel(pack, engine, callback, remover));
    }
    if (backbutton) {
        panel->add(guiutil::backButton(engine->getGUI()->getMenu()));
    }
    return panel;
}

static void reopen_world(Engine* engine, World* world) {
    std::string wname = world->wfile->directory.stem().u8string();
    engine->setScreen(nullptr);
    engine->setScreen(std::make_shared<MenuScreen>(engine));
    menus::open_world(wname, engine, true);
}

// FIXME
static bool try_add_dependency(Engine* engine, World* world, const ContentPack& pack, std::string& missing) {
    auto paths = engine->getPaths();
    for (const auto& dependency : pack.dependencies) {
        fs::path folder = ContentPack::findPack(
            paths, 
            world->wfile->directory, 
            dependency
        );
        if (!fs::is_directory(folder)) {
            missing = dependency;
            return true;
        }
        if (!world->hasPack(dependency)) {
            world->wfile->addPack(world, dependency);
        }
    }
    world->wfile->addPack(world, pack.id);
    return false;
}

void menus::remove_packs(
    Engine* engine,
    LevelController* controller,
    std::vector<std::string> packsToRemove
) {
    auto content = engine->getContent();
    auto world = controller->getLevel()->getWorld();
    bool hasIndices = false;

    std::stringstream ss;
    for (const auto& id : packsToRemove) {
        if (content->getPackRuntime(id)->getStats().hasSavingContent()) {
            if (hasIndices) {
                ss << ", ";
            }
            hasIndices = true;
            ss << id;
        }
    }

    runnable removeFunc = [=]() {
        controller->saveWorld();
        for (const auto& id : packsToRemove) {
            world->wfile->removePack(world, id);
        }
        reopen_world(engine, world);
    };

    if (hasIndices) {
        guiutil::confirm(
            engine->getGUI(), 
            langs::get(L"remove-confirm", L"pack")+
            L" ("+util::str2wstr_utf8(ss.str())+L")", 
            [=]() {removeFunc();}
        );
    } else {
        removeFunc();
    }
}

void create_content_panel(Engine* engine, LevelController* controller) {
    auto level = controller->getLevel();
    auto menu = engine->getGUI()->getMenu();
    auto mainPanel = menus::create_page(engine, "content", 550, 0.0f, 5);

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
    auto panel = menus::create_packs_panel(
        engine->getContentPacks(), engine, false, nullptr, 
        [=](const ContentPack& pack) {
            std::vector<std::string> packsToRemove {pack.id};
            menus::remove_packs(engine, controller, packsToRemove);
        }
    );
    mainPanel->add(panel);
    mainPanel->add(menus::create_button(
    langs::get(L"Add", L"content"), glm::vec4(10.0f), glm::vec4(1), [=](GUI* gui) {
        auto panel = menus::create_packs_panel(scanned, engine, true, 
        [=](const ContentPack& pack) {
            auto world = level->getWorld();
            std::string missing;
            if (try_add_dependency(engine, world, pack, missing)) {
                guiutil::alert(
                    gui, langs::get(L"error.dependency-not-found")+
                    L": "+util::str2wstr_utf8(missing)
                );
                return;
            }
            world->wfile->addPack(world, pack.id);
            controller->saveWorld();
            reopen_world(engine, world);
        }, nullptr);
        menu->addPage("content-packs", panel);
        menu->setPage("content-packs");
    }));
    mainPanel->add(guiutil::backButton(menu));
}

void menus::create_pause_panel(Engine* engine, LevelController* controller) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = create_page(engine, "pause", 400, 0.0f, 1);

    panel->add(create_button(L"Continue", glm::vec4(10.0f), glm::vec4(1), [=](GUI*){
        menu->reset();
    }));
    panel->add(create_button(L"Content", glm::vec4(10.0f), glm::vec4(1), [=](GUI*) {
        create_content_panel(engine, controller);
        menu->setPage("content");
    }));
    panel->add(guiutil::gotoButton(L"Settings", "settings", menu));

    panel->add(create_button(L"Save and Quit to Menu", glm::vec4(10.f), glm::vec4(1), [=](GUI*){
        engine->postRunnable([=]() {
            // save world
            controller->saveWorld();
            // destroy LevelScreen and run quit callbacks
            engine->setScreen(nullptr);
            // create and go to menu screen
            engine->setScreen(std::make_shared<MenuScreen>(engine));
        });
    }));
}
