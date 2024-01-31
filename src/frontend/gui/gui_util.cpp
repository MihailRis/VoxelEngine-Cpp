#include "gui_util.h"
#include "controls.h"
#include "panels.h"

#include <glm/glm.hpp>

#include "../locale/langs.h"

using namespace gui;
using glm::vec2;
using glm::vec4;

Button* guiutil::backButton(PagesControl* menu) {
    return (new Button(langs::get(L"Back"), vec4(10.f)))->listenAction([=](GUI* gui) {
        menu->back();
    });
}

Button* guiutil::gotoButton(
        std::wstring text, 
        const std::string& page, 
        PagesControl* menu) {
    text = langs::get(text, L"menu");
    return (new Button(text, vec4(10.f)))->listenAction([=](GUI* gui) {
        menu->set(page);
    });
}

void guiutil::alert(GUI* gui, const std::wstring& text, gui::runnable on_hidden) {
    PagesControl* menu = gui->getMenu();
    Panel* panel = new Panel(vec2(500, 200), vec4(8.0f), 8.0f);
    panel->setColor(vec4(0.0f, 0.0f, 0.0f, 0.5f));
    
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
            panel->add(new Label(part));
            offset += extra;
        }
    } else {
        panel->add(new Label(text));
    }
    panel->add((new Button(langs::get(L"Ok"), vec4(10.f)))->listenAction([=](GUI* gui) {
        if (on_hidden)
            on_hidden();
        menu->back();
    }));
    panel->refresh();
    menu->add("<alert>", panel);
    menu->set("<alert>");
}

void guiutil::confirm(
        GUI* gui, 
        const std::wstring& text, 
        gui::runnable on_confirm,
        std::wstring yestext, 
        std::wstring notext) {
    if (yestext.empty()) yestext = langs::get(L"Yes");
    if (notext.empty()) notext = langs::get(L"No");

    PagesControl* menu = gui->getMenu();
    Panel* panel = new Panel(vec2(600, 200), vec4(8.0f), 8.0f);
    panel->setColor(vec4(0.0f, 0.0f, 0.0f, 0.5f));
    panel->add(new Label(text));
    Panel* subpanel = new Panel(vec2(600, 53));
    subpanel->setColor(vec4(0));
    subpanel->add((new Button(yestext, vec4(8.0f)))->listenAction([=](GUI*){
        if (on_confirm)
            on_confirm();
        menu->back();
    }));
    subpanel->add((new Button(notext, vec4(8.0f)))->listenAction([=](GUI*){
        menu->back();
    }));
    panel->add(subpanel);

    panel->refresh();
    menu->add("<confirm>", panel);
    menu->set("<confirm>");
}
