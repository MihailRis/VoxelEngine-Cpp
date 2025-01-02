#include "gui_util.hpp"

#include "elements/Label.hpp"
#include "elements/Menu.hpp"
#include "elements/Button.hpp"
#include "elements/TextBox.hpp"
#include "gui_xml.hpp"

#include "logic/scripting/scripting.hpp"
#include "frontend/locale.hpp"
#include "util/stringutil.hpp"
#include "delegates.hpp"

#include "window/Events.hpp"
#include "engine/Engine.hpp"

#include <glm/glm.hpp>

using namespace gui;

std::shared_ptr<gui::UINode> guiutil::create(const std::string& source, scriptenv env) {
    if (env == nullptr) {
        env = scripting::get_root_environment();
    }
    UiXmlReader reader(env);
    return reader.readXML("[string]", source);
}

void guiutil::alert(
    const std::shared_ptr<gui::Menu>& menu,
    const std::wstring& text,
    const runnable& on_hidden
) {
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
    Engine& engine,
    const std::wstring& text,
    const runnable& on_confirm,
    const runnable& on_deny,
    std::wstring yestext,
    std::wstring notext
) {
    if (yestext.empty()) yestext = langs::get(L"Yes");
    if (notext.empty()) notext = langs::get(L"No");

    auto panel = std::make_shared<Panel>(glm::vec2(600, 200), glm::vec4(8.0f), 8.0f);
    panel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    panel->add(std::make_shared<Label>(text));
    auto subpanel = std::make_shared<Panel>(glm::vec2(600, 53));
    subpanel->setColor(glm::vec4(0));

    auto menu = engine.getGUI()->getMenu();

    runnable on_confirm_final = [on_confirm, menu, &engine]() {
        if (on_confirm) {
            on_confirm();
        }
        menu->back();
        engine.postRunnable([menu]() {
            menu->removePage("<confirm>");
        });
    };

    runnable on_deny_final = [on_deny, menu, &engine]() {
        if (on_deny) {
            on_deny();
        }
        menu->back();
        engine.postRunnable([menu]() {
            menu->removePage("<confirm>");
        });
    };

    subpanel->add(std::make_shared<Button>(yestext, glm::vec4(8.f), [=](GUI*){
        on_confirm_final();
    }));

    subpanel->add(std::make_shared<Button>(notext, glm::vec4(8.f), [=](GUI*){
        on_deny_final();
    }));

    panel->add(subpanel);
    panel->keepAlive(Events::keyCallbacks[keycode::ENTER].add([=](){
        on_confirm_final();
        return true;
    }));
    panel->keepAlive(Events::keyCallbacks[keycode::ESCAPE].add([=](){
        on_deny_final();
        return true;
    }));

    panel->refresh();
    menu->addPage("<confirm>", panel);
    menu->setPage("<confirm>");
}

void guiutil::confirm_with_memo(
        const std::shared_ptr<gui::Menu>& menu,
        const std::wstring& text, 
        const std::wstring& memo,
        const runnable& on_confirm,
        std::wstring yestext, 
        std::wstring notext) {

    if (yestext.empty()) yestext = langs::get(L"Yes");
    if (notext.empty()) notext = langs::get(L"No");

    auto panel = std::make_shared<Panel>(glm::vec2(600, 500), glm::vec4(8.0f), 8.0f);
    panel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    panel->add(std::make_shared<Label>(text));
    
    auto textbox = std::make_shared<TextBox>(L"");
    textbox->setMultiline(true);
    textbox->setTextWrapping(true);
    textbox->setSize(glm::vec2(600, 300));
    textbox->setText(memo);
    textbox->setEditable(false);
    panel->add(textbox);

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
