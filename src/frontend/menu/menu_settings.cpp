#include "menu.h"
#include "menu_commons.h"

#include "../locale/langs.h"
#include "../../graphics/ui/GUI.h"
#include "../../graphics/ui/gui_util.h"
#include "../../engine.h"
#include "../../util/stringutil.h"
#include "../../window/Events.h"

#include <glm/glm.hpp>

using namespace gui;

void menus::create_settings_panel(Engine* engine) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = menus::create_page(engine, "settings", 400, 0.0f, 1);

    /* V-Sync checkbox */{
        auto checkbox = std::make_shared<FullCheckBox>(
            langs::get(L"V-Sync", L"settings"), glm::vec2(400, 32)
        );
        checkbox->setSupplier([=]() {
            return engine->getSettings().display.swapInterval != 0;
        });
        checkbox->setConsumer([=](bool checked) {
            engine->getSettings().display.swapInterval = checked;
        });
        panel->add(checkbox);
    }

    /* Backlight checkbox */{
        auto checkbox = std::make_shared<FullCheckBox>(
            langs::get(L"Backlight", L"settings"), glm::vec2(400, 32)
        );
        checkbox->setSupplier([=]() {
            return engine->getSettings().graphics.backlight;
        });
        checkbox->setConsumer([=](bool checked) {
            engine->getSettings().graphics.backlight = checked;
        });
        panel->add(checkbox);
    }

    /* Camera shaking checkbox */ {
        auto checkbox = std::make_shared<FullCheckBox>(
            langs::get(L"Camera Shaking", L"settings"), glm::vec2(400, 32)
            );
        checkbox->setSupplier([=]() {
            return engine->getSettings().camera.shaking;
            });
        checkbox->setConsumer([=](bool checked) {
            engine->getSettings().camera.shaking = checked;
            });
        panel->add(checkbox);
    }

    std::string langName = langs::locales_info.at(langs::current->getId()).name;
    panel->add(guiutil::gotoButton(
        langs::get(L"Language", L"settings")+L": "+
        util::str2wstr_utf8(langName), 
        "languages", menu));

    panel->add(guiutil::gotoButton(L"Audio", "settings_audio", menu));
    panel->add(guiutil::gotoButton(L"Controls", "controls", menu));
    panel->add(guiutil::backButton(menu));
}
