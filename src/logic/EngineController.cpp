#include "EngineController.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>

#include "engine/Engine.hpp"
#include "coders/commons.hpp"
#include "debug/Logger.hpp"
#include "coders/json.hpp"
#include "content/ContentReport.hpp"
#include "files/WorldConverter.hpp"
#include "files/WorldFiles.hpp"
#include "frontend/locale.hpp"
#include "frontend/menu.hpp"
#include "frontend/screens/LevelScreen.hpp"
#include "frontend/screens/MenuScreen.hpp"
#include "graphics/ui/elements/Menu.hpp"
#include "graphics/ui/gui_util.hpp"
#include "objects/Players.hpp"
#include "interfaces/Task.hpp"
#include "util/stringutil.hpp"
#include "world/Level.hpp"
#include "world/World.hpp"
#include "LevelController.hpp"

namespace fs = std::filesystem;

static debug::Logger logger("engine-control");

EngineController::EngineController(Engine& engine) : engine(engine) {
}

void EngineController::deleteWorld(const std::string& name) {
    fs::path folder = engine.getPaths().getWorldFolderByName(name);

    auto deletion = [this, folder]() {
        logger.info() << "deleting " << folder;
        fs::remove_all(folder);
        if (!engine.isHeadless()) {
            engine.getGUI()->getMenu()->back();
        }
    };

    if (engine.isHeadless()) {
        deletion();
        return;
    }
    guiutil::confirm(
        engine,
        langs::get(L"delete-confirm", L"world") + L" (" +
            util::str2wstr_utf8(folder.u8string()) + L")",
        deletion
    );
}

std::shared_ptr<Task> create_converter(
    Engine& engine,
    const std::shared_ptr<WorldFiles>& worldFiles,
    const Content* content,
    const std::shared_ptr<ContentReport>& report,
    const runnable& postRunnable
) {
    ConvertMode mode;
    if (report->isUpgradeRequired()) {
        mode = ConvertMode::UPGRADE;
    } else if (report->hasContentReorder()) {
        mode = ConvertMode::REINDEX;
    } else {
        mode = ConvertMode::BLOCK_FIELDS;
    }
    return WorldConverter::startTask(
        worldFiles,
        content,
        report,
        [&engine, postRunnable]() {
            //auto menu = engine.getGUI()->getMenu();
            //menu->reset();
            //menu->setPage("main", false);
            engine.postRunnable([=]() { postRunnable(); });
        },
        mode,
        true
    );
}

static void show_convert_request(
    Engine& engine,
    const Content* content,
    const std::shared_ptr<ContentReport>& report,
    const std::shared_ptr<WorldFiles>& worldFiles,
    const runnable& postRunnable
) {
    auto on_confirm = [&engine, worldFiles, content, report, postRunnable]() {
            auto converter =
                create_converter(engine, worldFiles, content, report, postRunnable);
            menus::show_process_panel(
                engine, converter, L"Converting world..."
            );
        };

    std::wstring message = L"world.convert-block-layouts";
    if (report->hasContentReorder()) {
        message = L"world.convert-request";
    }
    if (report->isUpgradeRequired()) {
        message = L"world.upgrade-request";
    } else if (report->hasDataLoss()) {
        message = L"world.convert-with-loss";
        std::wstring text;
        for (const auto& line : report->getDataLoss()) {
            text += util::str2wstr_utf8(line) + L"\n";
        }
        guiutil::confirm_with_memo(
            engine.getGUI()->getMenu(),
            langs::get(message),
            text,
            on_confirm,
            L"",
            langs::get(L"Cancel")
        );
        return;
    }
    guiutil::confirm(
        engine,
        langs::get(message),
        on_confirm,
        nullptr,
        L"",
        langs::get(L"Cancel")
    );
}

static bool load_world_content(Engine& engine, const fs::path& folder) {
    if (engine.isHeadless()) {
        engine.loadWorldContent(folder);
        return true;
    } else {
        return menus::call(engine, [&engine, folder]() {
            engine.loadWorldContent(folder);
        });
    }
}

static void load_world(
    Engine& engine,
    const std::shared_ptr<WorldFiles>& worldFiles,
    int64_t localPlayer
) {
    try {
        auto content = engine.getContent();
        auto& packs = engine.getContentPacks();
        auto& settings = engine.getSettings();

        auto level = World::load(worldFiles, settings, *content, packs);
        engine.onWorldOpen(std::move(level), localPlayer);
    } catch (const world_load_error& error) {
        guiutil::alert(
            engine,
            langs::get(L"Error") + L": " + util::str2wstr_utf8(error.what())
        );
        return;
    }
}

static dv::value create_missing_content_report(
    const std::shared_ptr<ContentReport>& report
) {
    auto root = dv::object();
    auto& contentEntries = root.list("content");
    for (auto& entry : report->getMissingContent()) {
        std::string contentName = ContentType_name(entry.type);
        auto& contentEntry = contentEntries.object();
        contentEntry["type"] = contentName;
        contentEntry["name"] = entry.name;
    }
    return root;
}

void EngineController::onMissingContent(const std::shared_ptr<ContentReport>& report) {
    if (engine.isHeadless()) {
        throw std::runtime_error(
            "missing content: " +
            json::stringify(create_missing_content_report(report), true)
        );
    } else {
        engine.setScreen(std::make_shared<MenuScreen>(engine));
        menus::show(
            engine,
            "reports/missing_content",
            {create_missing_content_report(report)}
        );
    }
}

void EngineController::openWorld(const std::string& name, bool confirmConvert) {
    const auto& paths = engine.getPaths();
    auto folder = paths.getWorldsFolder() / fs::u8path(name);
    auto worldFile = folder / fs::u8path("world.json");
    if (!fs::exists(worldFile)) {
        throw std::runtime_error(worldFile.u8string() + " does not exists");
    }

    if (!load_world_content(engine, folder)) {
        return;
    }

    const Content* content = engine.getContent();
    auto worldFiles = std::make_shared<WorldFiles>(
        folder, engine.getSettings().debug);
    if (auto report = World::checkIndices(worldFiles, content)) {
        if (report->hasMissingContent()) {
            onMissingContent(report);
        } else {
            if (confirmConvert) {
                auto task = create_converter(
                    engine,
                    worldFiles,
                    content,
                    report,
                    [=]() { openWorld(name, false); }
                );
                if (engine.isHeadless()) {
                    task->waitForEnd();
                } else {
                    menus::show_process_panel(
                        engine, task, L"Converting world..."
                    );
                }
            } else {
                show_convert_request(engine, content, report, std::move(worldFiles), [=]() {
                    openWorld(name, false);
                });
            }
        }
        return;
    }
    load_world(engine, std::move(worldFiles), localPlayer);
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

    EnginePaths& paths = engine.getPaths();
    auto folder = paths.getWorldsFolder() / fs::u8path(name);

    if (engine.isHeadless()) {
        engine.loadContent();
        paths.setCurrentWorldFolder(folder);
    } else if (!menus::call(engine, [this, &paths, folder]() {
            engine.loadContent();
            paths.setCurrentWorldFolder(folder);
        })) {
        return;
    }
    auto level = World::create(
        name,
        generatorID,
        folder,
        seed,
        engine.getSettings(),
        *engine.getContent(),
        engine.getContentPacks()
    );
    if (!engine.isHeadless()) {
        level->players->create(localPlayer);
    }
    engine.onWorldOpen(std::move(level), localPlayer);
}

void EngineController::setLocalPlayer(int64_t player) {
    localPlayer = player;
}

void EngineController::reopenWorld(World* world) {
    std::string name = world->wfile->getFolder().filename().u8string();
    engine.onWorldClosed();
    openWorld(name, true);
}

void EngineController::reconfigPacks(
    LevelController* controller,
    const std::vector<std::string>& packsToAdd,
    const std::vector<std::string>& packsToRemove
) {
    auto content = engine.getContent();
    bool hasIndices = false;

    std::stringstream ss;
    if (content) {
        for (const auto& id : packsToRemove) {
            auto runtime = content->getPackRuntime(id);
            if (runtime && runtime->getStats().hasSavingContent()) {
                if (hasIndices) {
                    ss << ", ";
                }
                hasIndices = true;
                ss << id;
            }
        }
    }

    runnable removeFunc = [this, controller, packsToAdd, packsToRemove]() {
        if (controller == nullptr) {
            try {
                auto manager = engine.createPacksManager(fs::path(""));
                manager.scan();
                auto names = PacksManager::getNames(engine.getContentPacks());
                for (const auto& id : packsToAdd) {
                    names.push_back(id);
                }
                for (const auto& id : packsToRemove) {
                    manager.exclude(id);
                    names.erase(std::find(names.begin(), names.end(), id));
                }
                names = manager.assemble(names);
                engine.getContentPacks() = manager.getAll(names);
            } catch (const contentpack_error& err) {
                throw std::runtime_error(
                    std::string(err.what()) + " [" + err.getPackId() + "]"
                );
            }
        } else {
            auto world = controller->getLevel()->getWorld();
            auto& wfile = *world->wfile;
            controller->saveWorld();
            auto manager = engine.createPacksManager(wfile.getFolder());
            manager.scan();

            auto names = PacksManager::getNames(world->getPacks());
            for (const auto& id : packsToAdd) {
                names.push_back(id);
            }
            for (const auto& id : packsToRemove) {
                manager.exclude(id);
                const auto& found = std::find(names.begin(), names.end(), id);
                if (found != names.end()) {
                    names.erase(found);
                } else {
                    logger.warning()
                        << "attempt to remove non-installed pack: " << id;
                }
            }
            wfile.removeIndices(packsToRemove);
            wfile.writePacks(manager.getAll(names));
            reopenWorld(world);
        }
    };

    if (hasIndices && !engine.isHeadless()) {
        guiutil::confirm(
            engine,
            langs::get(L"remove-confirm", L"pack") + L" (" +
                util::str2wstr_utf8(ss.str()) + L")",
            [=]() { removeFunc(); }
        );
    } else {
        removeFunc();
    }
}
