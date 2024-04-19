#ifndef FRONTEND_MENU_MENU_CREATE_WORLD_H_
#define FRONTEND_MENU_MENU_CREATE_WORLD_H_

#include "../../engine.h"
#include "../../files/WorldFiles.h"
#include "../../content/PacksManager.h"
#include "../../graphics/ui/elements/containers.h"
#include "../../graphics/ui/elements/controls.h"
#include "../../graphics/ui/gui_util.h"
#include "../../util/stringutil.h"
#include "../../world/Level.h"
#include "../../world/World.h"
#include "../../world/WorldGenerators.h"
#include "../locale/langs.h"
#include "../screens.h"
#include "menu_commons.h"
#include "menu.h"

#include <iostream>

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

#endif // FRONTEND_MENU_MENU_CREATE_WORLD_H_
