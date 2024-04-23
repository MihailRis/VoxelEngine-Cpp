#include "menu.hpp"

#include <filesystem>
#include <glm/glm.hpp>

#include "../delegates.h"
#include "../engine.h"
#include "../data/dynamic.h"
#include "../interfaces/Task.h"
#include "../files/engine_paths.h"
#include "../graphics/ui/elements/Label.hpp"
#include "../graphics/ui/elements/Panel.hpp"
#include "../graphics/ui/elements/Menu.hpp"
#include "../graphics/ui/gui_util.h"
#include "../graphics/ui/GUI.h"
#include "../logic/scripting/scripting.h"
#include "../settings.h"
#include "../util/stringutil.h"
#include "../window/Window.h"
#include "locale/langs.h"
#include "UiDocument.h"

namespace fs = std::filesystem;
using namespace gui;

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

void menus::create_menus(Engine* engine) {
    auto menu = engine->getGUI()->getMenu();
    menu->setPageLoader([=](auto name) {
        auto file = engine->getResPaths()->find("layouts/pages/"+name+".xml");
        auto fullname = "core:pages/"+name;

        auto document = UiDocument::read(scripting::get_root_environment(), fullname, file).release();
        engine->getAssets()->store(document, fullname);
        scripting::on_ui_open(document, {});
        return document->getRoot();
    });
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
