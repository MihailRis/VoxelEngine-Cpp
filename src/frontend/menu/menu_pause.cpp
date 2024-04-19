#include "menu.h"
#include "menu_commons.h"

// TODO: move functions to EngineController

#include "../../content/PacksManager.h"
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

inline uint64_t randU64() {
    srand(time(NULL));
    return rand() ^ (rand() << 8) ^ 
        (rand() << 16) ^ (rand() << 24) ^
        ((uint64_t)rand() << 32) ^ 
        ((uint64_t)rand() << 40) ^
        ((uint64_t)rand() << 56);
}

inline uint64_t str2seed(const std::string& seedstr) {
    if (util::is_integer(seedstr)) {
        try {
            return std::stoull(seedstr);
        } catch (const std::out_of_range& err) {
            std::hash<std::string> hash;
            return hash(seedstr);
        }
    } else {
        std::hash<std::string> hash;
        return hash(seedstr);
    }
}

void menus::create_world(
    Engine* engine, 
    const std::string& name, 
    const std::string& seedstr,
    const std::string& generatorID
) {
    uint64_t seed = str2seed(seedstr);

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
        name, generatorID, folder, seed, 
        engine->getSettings(), 
        engine->getContent(),
        engine->getContentPacks()
    );
    level->getWorld()->wfile->createDirectories();
    engine->setScreen(std::make_shared<LevelScreen>(engine, level));
}

static void reopen_world(Engine* engine, World* world) {
    std::string wname = world->wfile->getFolder().filename().u8string();
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
