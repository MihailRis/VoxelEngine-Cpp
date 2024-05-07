#include "EngineController.hpp"

#include "../content/ContentLUT.hpp"
#include "../debug/Logger.hpp"
#include "../engine.hpp"
#include "../files/WorldFiles.hpp"
#include "../files/WorldConverter.hpp"
#include "../frontend/locale.hpp"
#include "../frontend/screens/MenuScreen.hpp"
#include "../frontend/screens/LevelScreen.hpp"
#include "../frontend/menu.hpp"
#include "../graphics/ui/elements/Menu.hpp"
#include "../graphics/ui/gui_util.hpp"
#include "../interfaces/Task.hpp"
#include "../util/stringutil.hpp"
#include "../world/World.hpp"
#include "../world/Level.hpp"
#include "LevelController.hpp"

#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

static debug::Logger logger("engine-control");

EngineController::EngineController(Engine* engine) : engine(engine) {
}

void EngineController::deleteWorld(std::string name) {
    fs::path folder = engine->getPaths()->getWorldFolder(name);
    guiutil::confirm(engine->getGUI(), langs::get(L"delete-confirm", L"world")+
    L" ("+util::str2wstr_utf8(folder.u8string())+L")", [=]() {
        logger.info() << "deleting " << folder.u8string();
        fs::remove_all(folder);
    });
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
        menus::show_process_panel(engine, converter, L"Converting world...");
    }, L"", langs::get(L"Cancel"));
}

static void show_content_missing(
    Engine* engine,
    const Content* content,
    std::shared_ptr<ContentLUT> lut
) {
    using namespace dynamic;
    auto root = std::make_unique<Map>();
    auto& contentEntries = root->putList("content");
    for (auto& entry : lut->getMissingContent()) {
        std::string contentName = contenttype_name(entry.type);
        auto& contentEntry = contentEntries.putMap();
        contentEntry.put("type", contentName);
        contentEntry.put("name", entry.name);
    }
    std::vector<std::unique_ptr<dynamic::Value>> args;
    args.emplace_back(std::make_unique<Value>(root.release()));
    menus::show(engine, "reports/missing_content", std::move(args));
}

static bool loadWorldContent(Engine* engine, fs::path folder) {
    try {
        engine->loadWorldContent(folder);
        return true;
    } catch (const contentpack_error& error) {
        engine->setScreen(std::make_shared<MenuScreen>(engine));
        // could not to find or read pack
        guiutil::alert(
            engine->getGUI(), langs::get(L"error.pack-not-found")+L": "+
            util::str2wstr_utf8(error.getPackId())
        );
        return false;
    } catch (const std::runtime_error& error) {
        engine->setScreen(std::make_shared<MenuScreen>(engine));
        guiutil::alert(
            engine->getGUI(), langs::get(L"Content Error", L"menu")+L": "+
            util::str2wstr_utf8(error.what())
        );
        return false;
    }
}

static void loadWorld(Engine* engine, fs::path folder) {
    try {
        auto content = engine->getContent();
        auto& packs = engine->getContentPacks();
        auto& settings = engine->getSettings();

        Level* level = World::load(folder, settings, content, packs);
        engine->setScreen(std::make_shared<LevelScreen>(engine, level));
    } catch (const world_load_error& error) {
        guiutil::alert(
            engine->getGUI(), langs::get(L"Error")+L": "+
            util::str2wstr_utf8(error.what())
        );
        return;
    }
}

void EngineController::openWorld(std::string name, bool confirmConvert) {
    auto paths = engine->getPaths();
    auto folder = paths->getWorldsFolder()/fs::u8path(name);
    if (!loadWorldContent(engine, folder)) {
        return;
    }

    auto* content = engine->getContent();

    std::shared_ptr<ContentLUT> lut (World::checkIndices(folder, content));
    if (lut) {
        if (lut->hasMissingContent()) {
            engine->setScreen(std::make_shared<MenuScreen>(engine));
            show_content_missing(engine, content, lut);
        } else {
            if (confirmConvert) {
                menus::show_process_panel(engine, create_converter(engine, folder, content, lut, [=]() {
                    openWorld(name, false);
                }), L"Converting world...");
            } else {
                show_convert_request(engine, content, lut, folder, [=](){
                    openWorld(name, false);
                });
            }
        }
    } else {
        loadWorld(engine, folder);
    }
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

void EngineController::createWorld(
    const std::string& name, 
    const std::string& seedstr,
    const std::string& generatorID
) {
    uint64_t seed = str2seed(seedstr);

    EnginePaths* paths = engine->getPaths();
    auto folder = paths->getWorldsFolder()/fs::u8path(name);
    try {
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
    engine->setScreen(std::make_shared<LevelScreen>(engine, level));
}

void EngineController::reopenWorld(World* world) {
    std::string wname = world->wfile->getFolder().filename().u8string();
    engine->setScreen(nullptr);
    engine->setScreen(std::make_shared<MenuScreen>(engine));
    openWorld(wname, true);
}

void EngineController::reconfigPacks(
    LevelController* controller,
    std::vector<std::string> packsToAdd,
    std::vector<std::string> packsToRemove
) {
    auto content = engine->getContent();
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
        if (controller == nullptr) {
            auto manager = engine->createPacksManager(fs::path(""));
            manager.scan();
            std::vector<std::string> names = engine->getBasePacks();
            for (auto& name : packsToAdd) {
                names.push_back(name);
            }
            engine->getContentPacks() = manager.getAll(names);
        } else {
            auto world = controller->getLevel()->getWorld();
            auto wfile = world->wfile.get();
            controller->saveWorld();
            auto manager = engine->createPacksManager(wfile->getFolder());
            manager.scan();

            auto names = PacksManager::getNames(world->getPacks());
            for (const auto& id : packsToAdd) {
                names.push_back(id);
            }
            for (const auto& id : packsToRemove) {
                manager.exclude(id);
                names.erase(std::find(names.begin(), names.end(), id));
            }
            wfile->removeIndices(packsToRemove);
            wfile->writePacks(manager.getAll(names));
            reopenWorld(world);
        }
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
