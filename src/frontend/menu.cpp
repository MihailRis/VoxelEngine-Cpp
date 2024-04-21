#include "menu.hpp"

#include <filesystem>
#include <glm/glm.hpp>

#include "../delegates.h"
#include "../engine.h"
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
        scripting::on_ui_open(document, nullptr, glm::ivec3());
        return document->getRoot();
    });
}

void menus::show_process_panel(Engine* engine, std::shared_ptr<Task> task, std::wstring text) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = std::dynamic_pointer_cast<gui::Panel>(guiutil::create(
        "<panel size='400' padding='8' interval='1' color='#00000080/>"
    ));
    if (!text.empty()) {
        panel->add(std::make_shared<gui::Label>(langs::get(text)));
    }

    auto label = std::make_shared<gui::Label>(L"0%");
    panel->add(label);

    uint initialWork = task->getWorkTotal();

    panel->listenInterval(0.01f, [=]() {
        task->update();

        uint tasksDone = task->getWorkDone();
        float progress = tasksDone/static_cast<float>(initialWork);
        label->setText(
            std::to_wstring(tasksDone)+
            L"/"+std::to_wstring(initialWork)+L" ("+
            std::to_wstring(static_cast<int>(progress*100))+L"%)"
        );
    });

    menu->reset();
    menu->addPage("process", panel);
    menu->setPage("process", false);
}
