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

std::shared_ptr<gui::Button> generatorTypeButton;

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

void menus::create_world_generators_panel(Engine* engine) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = menus::create_page(engine, "world_generators", 400, 0.5f, 1);
    panel->setScrollable(true);

    std::vector<std::string> generatorsIDs = WorldGenerators::getGeneratorsIDs();
    std::sort(generatorsIDs.begin(), generatorsIDs.end());
    for (std::string& id : generatorsIDs) {
        const std::string& fullName = translate_generator_id(id);
        auto button = std::make_shared<Container>(glm::vec2(80, 30));

        auto idlabel = std::make_shared<Label>("["+id+"]");
        idlabel->setColor(glm::vec4(1, 1, 1, 0.5f));
        idlabel->setSize(glm::vec2(300, 25));
        idlabel->setAlign(Align::right);

        button->add(idlabel, glm::vec2(80, 4));
        button->add(std::make_shared<Label>(fullName), glm::vec2(0, 8));
        button->listenAction(
            [=](GUI*) {
                menus::generatorID = id;
                generatorTypeButton->setText(langs::get(L"World generator", L"world") + (L": ") + util::str2wstr_utf8(translate_generator_id(menus::generatorID)));
                menu->back();
            }
        );
        panel->add(button);
    }
    panel->add(guiutil::backButton(menu));
}

void menus::create_new_world_panel(Engine* engine) {
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
    
    generatorTypeButton = guiutil::gotoButton(langs::get(L"World generator", L"world") + (L": ") + util::str2wstr_utf8(translate_generator_id(menus::generatorID)), "world_generators", engine->getGUI()->getMenu());
    panel->add(generatorTypeButton);

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

#endif // FRONTEND_MENU_MENU_CREATE_WORLD_H_
