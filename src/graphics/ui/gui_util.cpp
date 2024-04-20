#include "gui_util.h"
#include "elements/display/Label.hpp"
#include "elements/layout/Menu.hpp"
#include "elements/control/Button.hpp"
#include "gui_xml.h"

#include <glm/glm.hpp>

#include "../../logic/scripting/Environment.h"
#include "../../frontend/locale/langs.h"
#include "../../util/stringutil.h"
#include "../../delegates.h"

using namespace gui;

std::shared_ptr<Button> guiutil::backButton(std::shared_ptr<Menu> menu) {
    return std::dynamic_pointer_cast<Button>(create(
        "<button padding='10' onclick='menu:back()'>@Back</button>"
    ));
}

std::shared_ptr<Button> guiutil::gotoButton(
    std::wstring text, 
    const std::string& page, 
    std::shared_ptr<Menu> menu
) {
    text = langs::get(text, L"menu");
    return std::dynamic_pointer_cast<Button>(create(
        "<button onclick='menu.page=\""+page+"\"' padding='10'>"+
            util::wstr2str_utf8(text)+
        "</button>"
    ));
}

std::shared_ptr<gui::UINode> guiutil::create(const std::string& source, int envid) {
    scripting::Environment env(envid);
    UiXmlReader reader(env);
    auto node = reader.readXML("<string>", source);
    env.release();
    return node;
}

void guiutil::alert(GUI* gui, const std::wstring& text, runnable on_hidden) {
    auto menu = gui->getMenu();
    auto panel = std::make_shared<Panel>(glm::vec2(500, 200), glm::vec4(8.0f), 8.0f);
    panel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    
    // TODO: implement built-in text wrapping
    const int wrap_length = 60;
    if (text.length() > wrap_length) {
        size_t offset = 0;
        int extra;
        while ((extra = text.length() - offset) > 0) {
            size_t endline = text.find(L'\n', offset);
            if (endline != std::string::npos) {
                extra = std::min(extra, int(endline-offset)+1);
            }
            extra = std::min(extra, wrap_length);
            std::wstring part = text.substr(offset, extra);
            panel->add(std::make_shared<Label>(part));
            offset += extra;
        }
    } else {
        panel->add(std::make_shared<Label>(text));
    }
    panel->add(std::make_shared<Button>(
        langs::get(L"Ok"), glm::vec4(10.f), 
        [=](GUI* gui) {
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
        runnable on_confirm,
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
