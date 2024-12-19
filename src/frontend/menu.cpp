#include "menu.hpp"

#include "locale.hpp"
#include "UiDocument.hpp"
#include "screens/MenuScreen.hpp"

#include "delegates.hpp"
#include "engine.hpp"
#include "data/dv.hpp"
#include "interfaces/Task.hpp"
#include "files/engine_paths.hpp"
#include "graphics/ui/elements/Menu.hpp"
#include "graphics/ui/gui_util.hpp"
#include "graphics/ui/GUI.hpp"
#include "logic/scripting/scripting.hpp"
#include "settings.hpp"
#include "coders/commons.hpp"
#include "util/stringutil.hpp"
#include "window/Window.hpp"

#include <filesystem>
#include <glm/glm.hpp>

namespace fs = std::filesystem;
using namespace gui;

void menus::create_version_label(Engine& engine) {
    auto gui = engine.getGUI();
    auto text = ENGINE_VERSION_STRING+" debug build";
    gui->add(guiutil::create(
        "<label z-index='1000' color='#FFFFFF80' gravity='top-right' margin='4'>"
        +text+
        "</label>"
    ));
}

gui::page_loader_func menus::create_page_loader(Engine& engine) {
    return [&](const std::string& query) {
        std::vector<dv::value> args;

        std::string name;
        size_t index = query.find('?');
        if (index != std::string::npos) {
            auto argstr = query.substr(index+1);
            name = query.substr(0, index);
            
            auto map = dv::object();
            auto filename = "query for "+name;
            BasicParser parser(filename, argstr);
            while (parser.hasNext()) {
                auto key = std::string(parser.readUntil('='));
                parser.nextChar();
                auto value = std::string(parser.readUntil('&'));
                map[key] = value;
            }
            args.emplace_back(map);
        } else {
            name = query;
        }

        auto file = engine.getResPaths()->find("layouts/pages/"+name+".xml");
        auto fullname = "core:pages/"+name;

        auto documentPtr = UiDocument::read(
            scripting::get_root_environment(),
            fullname,
            file,
            "core:layouts/pages/" + name
        );
        auto document = documentPtr.get();
        engine.getAssets()->store(std::move(documentPtr), fullname);
        scripting::on_ui_open(document, std::move(args));
        return document->getRoot();
    };
}

bool menus::call(Engine& engine, runnable func) {
    if (engine.isHeadless()) {
        throw std::runtime_error("menus::call(...) in headless mode");
    }
    auto gui = engine.getGUI();
    try {
        func();
        return true;
    } catch (const contentpack_error& error) {
        engine.setScreen(std::make_shared<MenuScreen>(engine));
        // could not to find or read pack
        guiutil::alert(
            gui, langs::get(L"error.pack-not-found")+L": "+
            util::str2wstr_utf8(error.getPackId())
        );
        return false;
    } catch (const assetload::error& error) {
        engine.setScreen(std::make_shared<MenuScreen>(engine));
        guiutil::alert(
            gui, langs::get(L"Assets Load Error", L"menu")+L":\n"+
            util::str2wstr_utf8(error.what())
        );
        return false;
    } catch (const parsing_error& error) {
        engine.setScreen(std::make_shared<MenuScreen>(engine));
        guiutil::alert(gui, util::str2wstr_utf8(error.errorLog()));
        return false;
    } catch (const std::runtime_error& error) {
        engine.setScreen(std::make_shared<MenuScreen>(engine));
        guiutil::alert(
            gui, langs::get(L"Content Error", L"menu")+L":\n"+
            util::str2wstr_utf8(error.what())
        );
        return false;
    }
}

UiDocument* menus::show(Engine& engine, const std::string& name, std::vector<dv::value> args) {
    auto menu = engine.getGUI()->getMenu();
    auto file = engine.getResPaths()->find("layouts/"+name+".xml");
    auto fullname = "core:layouts/"+name;

    auto documentPtr = UiDocument::read(
        scripting::get_root_environment(), fullname, file, "core:layouts/"+name
    );
    auto document = documentPtr.get();
    engine.getAssets()->store(std::move(documentPtr), fullname);
    scripting::on_ui_open(document, std::move(args));
    menu->addPage(name, document->getRoot());
    menu->setPage(name);
    return document;
}

void menus::show_process_panel(Engine& engine, const std::shared_ptr<Task>& task, const std::wstring& text) {
    uint initialWork = task->getWorkTotal();

    auto menu = engine.getGUI()->getMenu();
    menu->reset();
    auto doc = menus::show(engine, "process", {
        util::wstr2str_utf8(langs::get(text))
    });
    std::dynamic_pointer_cast<Container>(doc->getRoot())->listenInterval(0.01f, [=]() {
        task->update();

        uint tasksDone = task->getWorkDone();
        scripting::on_ui_progress(doc, tasksDone, initialWork);
    });
}
