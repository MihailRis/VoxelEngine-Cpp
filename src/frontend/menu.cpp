#include "menu.hpp"

#include "locale.hpp"
#include "UiDocument.hpp"

#include "../delegates.h"
#include "../engine.h"
#include "../data/dynamic.h"
#include "../interfaces/Task.hpp"
#include "../files/engine_paths.h"
#include "../graphics/ui/elements/Menu.hpp"
#include "../graphics/ui/gui_util.hpp"
#include "../graphics/ui/GUI.hpp"
#include "../logic/scripting/scripting.h"
#include "../settings.h"
#include "../coders/commons.hpp"
#include "../util/stringutil.hpp"
#include "../window/Window.hpp"

#include <filesystem>
#include <glm/glm.hpp>

namespace fs = std::filesystem;
using namespace gui;

void menus::create_version_label(Engine* engine) {
    auto gui = engine->getGUI();
    auto text = ENGINE_VERSION_STRING+" debug build";
    gui->add(guiutil::create(
        "<label z-index='1000' color='#FFFFFF80' gravity='bottom-left' margin='4'>"
        +text+
        "</label>"
    ));
}

gui::page_loader_func menus::create_page_loader(Engine* engine) {
    return [=](const std::string& query) {
        using namespace dynamic;

        std::vector<std::unique_ptr<Value>> args;

        std::string name;
        size_t index = query.find('?');
        if (index != std::string::npos) {
            auto argstr = query.substr(index+1);
            name = query.substr(0, index);
            
            auto map = std::make_unique<Map>();
            BasicParser parser("query for "+name, argstr);
            while (parser.hasNext()) {
                auto key = parser.readUntil('=');
                parser.nextChar();
                auto value = parser.readUntil('&');
                map->put(key, value);
            }
            args.push_back(Value::of(std::move(map)));
        } else {
            name = query;
        }

        auto file = engine->getResPaths()->find("layouts/pages/"+name+".xml");
        auto fullname = "core:pages/"+name;

        auto document = UiDocument::read(scripting::get_root_environment(), fullname, file).release();
        engine->getAssets()->store(document, fullname);

        scripting::on_ui_open(document, std::move(args));
        return document->getRoot();
    };
}

UiDocument* menus::show(Engine* engine, const std::string& name, std::vector<std::unique_ptr<dynamic::Value>> args) {
    auto menu = engine->getGUI()->getMenu();
    auto file = engine->getResPaths()->find("layouts/"+name+".xml");
    auto fullname = "core:layouts/"+name;

    auto document = UiDocument::read(scripting::get_root_environment(), fullname, file).release();
    engine->getAssets()->store(document, fullname);
    scripting::on_ui_open(document, std::move(args));
    menu->addPage(name, document->getRoot());
    menu->setPage(name);
    return document;
}

void menus::show_process_panel(Engine* engine, std::shared_ptr<Task> task, std::wstring text) {
    using namespace dynamic;
    
    uint initialWork = task->getWorkTotal();

    auto menu = engine->getGUI()->getMenu();
    menu->reset();
    std::vector<std::unique_ptr<dynamic::Value>> args;
    args.emplace_back(Value::of(util::wstr2str_utf8(langs::get(text))));
    auto doc = menus::show(engine, "process", std::move(args));
    std::dynamic_pointer_cast<Container>(doc->getRoot())->listenInterval(0.01f, [=]() {
        task->update();

        uint tasksDone = task->getWorkDone();
        scripting::on_ui_progress(doc, tasksDone, initialWork);
    });
}
