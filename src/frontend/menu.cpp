#include "menu.hpp"

#include <string>
#include <memory>
#include <filesystem>
#include <glm/glm.hpp>

#include "../delegates.h"
#include "../engine.h"
#include "../files/engine_paths.h"
#include "../graphics/ui/elements/display/Label.hpp"
#include "../graphics/ui/elements/layout/Menu.hpp"
#include "../graphics/ui/gui_util.h"
#include "../graphics/ui/GUI.h"
#include "../logic/scripting/scripting.h"
#include "../settings.h"
#include "../util/stringutil.h"
#include "../window/Window.h"
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

        auto document = UiDocument::read(0, fullname, file).release();
        engine->getAssets()->store(document, fullname);
        scripting::on_ui_open(document, nullptr, glm::ivec3());
        return document->getRoot();
    });
}
