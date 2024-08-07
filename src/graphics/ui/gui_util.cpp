#include "gui_util.hpp"

#include "elements/Label.hpp"
#include "elements/Menu.hpp"
#include "elements/Button.hpp"
#include "gui_xml.hpp"

#include <logic/scripting/scripting.hpp>
#include <frontend/locale.hpp>
#include <util/stringutil.hpp>
#include <delegates.hpp>

#include <glm/glm.hpp>

using namespace gui;

std::shared_ptr<gui::UINode> guiutil::create(const std::string& source, scriptenv env) {
    if (env == nullptr) {
        env = scripting::get_root_environment();
    }
    UiXmlReader reader(env);
    return reader.readXML("<string>", source);
}

void guiutil::alert(GUI* gui, const std::wstring& text, const runnable& on_hidden) {
    auto menu = gui->getMenu();
    auto panel = std::make_shared<Panel>(glm::vec2(500, 300), glm::vec4(8.0f), 8.0f);
    panel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    
    auto label = std::make_shared<Label>(text);
    label->setMultiline(true);
    label->setSize(glm::vec2(1, 90));
    panel->add(label);
    panel->add(std::make_shared<Button>(
        langs::get(L"Ok"), glm::vec4(10.f), 
        [=](GUI*) {
            if (on_hidden) {
                on_hidden();
            }
            menu->back();
        }
    ));
    panel->refresh();
    menu->addPage("<alert>", panel);
    menu->setPage("<alert>");
}

void guiutil::confirm(
        GUI* gui, 
        const std::wstring& text, 
        const runnable& on_confirm,
        std::wstring yestext, 
        std::wstring notext) {
    if (yestext.empty()) yestext = langs::get(L"Yes");
    if (notext.empty()) notext = langs::get(L"No");

    auto menu = gui->getMenu();
    auto panel = std::make_shared<Panel>(glm::vec2(600, 200), glm::vec4(8.0f), 8.0f);
    panel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    panel->add(std::make_shared<Label>(text));
    auto subpanel = std::make_shared<Panel>(glm::vec2(600, 53));
    subpanel->setColor(glm::vec4(0));

    subpanel->add(std::make_shared<Button>(yestext, glm::vec4(8.f), [=](GUI*){
        if (on_confirm)
            on_confirm();
        menu->back();
    }));

    subpanel->add(std::make_shared<Button>(notext, glm::vec4(8.f), [=](GUI*){
        menu->back();
    }));

    panel->add(subpanel);

    panel->refresh();
    menu->addPage("<confirm>", panel);
    menu->setPage("<confirm>");
}
