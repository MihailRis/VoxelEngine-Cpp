#include "menu.h"
#include "menu_commons.h"

#include "../../coders/imageio.h"
#include "../../content/PacksManager.h"
#include "../../content/ContentLUT.h"
#include "../../engine.h"
#include "../../files/WorldFiles.h"
#include "../../graphics/core/Texture.h"
#include "../../graphics/ui/gui_util.h"
#include "../../logic/LevelController.h"
#include "../../util/stringutil.h"
#include "../../world/Level.h"
#include "../../world/World.h"
#include "../locale/langs.h"
#include "../screens.h"

#include <glm/glm.hpp>

using namespace gui;

static void reopen_world(Engine* engine, World* world) {
    std::string wname = world->wfile->getFolder().stem().u8string();
    engine->setScreen(nullptr);
    engine->setScreen(std::make_shared<MenuScreen>(engine));
    menus::open_world(wname, engine, true);
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
        auto manager = engine->createPacksManager(world->wfile->getFolder());
        manager.scan();

        auto names = PacksManager::getNames(world->getPacks());
        for (const auto& id : packsToRemove) {
            manager.exclude(id);
            names.erase(std::find(names.begin(), names.end(), id));
        }
        world->wfile->removeIndices(packsToRemove);
        world->wfile->writePacks(manager.getAll(names));
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

void menus::add_packs(
    Engine* engine,
    LevelController* controller,
    std::vector<std::string> packs
) {
    auto level = controller->getLevel();
    auto gui = engine->getGUI();
    auto world = level->getWorld();
    auto new_packs = PacksManager::getNames(world->getPacks());
    for (auto& id : packs) {
        new_packs.push_back(id);
    }

    auto manager = engine->createPacksManager(world->wfile->getFolder());
    manager.scan();
    try {
        new_packs = manager.assembly(new_packs);
    } catch (const contentpack_error& err) {
        guiutil::alert(
            gui, langs::get(L"error.dependency-not-found")+
            L": "+util::str2wstr_utf8(err.getPackId())
        );
        return;
    }
    world->wfile->writePacks(manager.getAll(new_packs));
    controller->saveWorld();
    reopen_world(engine, world);
}

void menus::create_pause_panel(Engine* engine, LevelController* controller) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = create_page(engine, "pause", 400, 0.0f, 1);

    panel->add(create_button(L"Continue", glm::vec4(10.0f), glm::vec4(1), [=](GUI*){
        menu->reset();
    }));
    panel->add(create_button(L"Content", glm::vec4(10.0f), glm::vec4(1), [=](GUI*) {
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
