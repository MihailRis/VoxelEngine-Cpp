#include "menu.h"
#include "menu_commons.h"

#include "../locale/langs.h"
#include "../gui/GUI.h"
#include "../gui/gui_util.h"
#include "../../engine.h"
#include "../../util/stringutil.h"

#include <glm/glm.hpp>

using namespace gui;

void menus::create_settings_panel(Engine* engine) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = menus::create_page(engine, "settings", 400, 0.0f, 1);

    /* Load Distance setting track bar */{
        panel->add(menus::create_label([=]() {
            return langs::get(L"Load Distance", L"settings")+L": " + 
                std::to_wstring(engine->getSettings().chunks.loadDistance);
        }));

        auto trackbar = std::make_shared<TrackBar>(3, 66, 10, 1, 3);
        trackbar->setSupplier([=]() {
            return engine->getSettings().chunks.loadDistance;
        });
        trackbar->setConsumer([=](double value) {
            engine->getSettings().chunks.loadDistance = static_cast<uint>(value);
        });
        panel->add(trackbar);
    }

    /* Load Speed setting track bar */{
        panel->add(menus::create_label([=]() {
            return langs::get(L"Load Speed", L"settings")+L": " + 
                std::to_wstring(engine->getSettings().chunks.loadSpeed);
        }));

        auto trackbar = std::make_shared<TrackBar>(1, 32, 10, 1, 1);
        trackbar->setSupplier([=]() {
            return engine->getSettings().chunks.loadSpeed;
        });
        trackbar->setConsumer([=](double value) {
            engine->getSettings().chunks.loadSpeed = static_cast<uint>(value);
        });
        panel->add(trackbar);
    }

    /* Fog Curve setting track bar */{
        panel->add(menus::create_label([=]() {
            float value = engine->getSettings().graphics.fogCurve;
            return langs::get(L"Fog Curve", L"settings")+L": " +
                   util::to_wstring(value, 1);
        }));

        auto trackbar = std::make_shared<TrackBar>(1.0, 6.0, 1.0, 0.1, 2);
        trackbar->setSupplier([=]() {
            return engine->getSettings().graphics.fogCurve;
        });
        trackbar->setConsumer([=](double value) {
            engine->getSettings().graphics.fogCurve = value;
        });
        panel->add(trackbar);
    }

    /* Fov setting track bar */{
        panel->add(menus::create_label([=]() {
            int fov = (int)engine->getSettings().camera.fov;
            return langs::get(L"FOV", L"settings")+L": "+std::to_wstring(fov)+L"°";
        }));

        auto trackbar = std::make_shared<TrackBar>(30.0, 120.0, 90, 1, 4);
        trackbar->setSupplier([=]() {
            return engine->getSettings().camera.fov;
        });
        trackbar->setConsumer([=](double value) {
            engine->getSettings().camera.fov = value;
        });
        panel->add(trackbar);
    }

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

    panel->add(guiutil::gotoButton(L"Controls", "controls", menu));
    panel->add(guiutil::backButton(menu));
}
