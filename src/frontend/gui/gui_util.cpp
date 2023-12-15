#include "gui_util.h"
#include "controls.h"
#include "panels.h"

#include <glm/glm.hpp>

#include "../locale/langs.h"

using namespace gui;
using glm::vec2;
using glm::vec4;
using std::string;
using std::wstring;

Button* guiutil::backButton(PagesControl* menu) {
    return (new Button(langs::get(L"Back"), vec4(10.f)))->listenAction([=](GUI* gui) {
        menu->back();
    });
}

Button* guiutil::gotoButton(wstring text, string page, PagesControl* menu) {
    return (new Button(text, vec4(10.f)))->listenAction([=](GUI* gui) {
        menu->set(page);
    });
}

void guiutil::alert(GUI* gui, wstring text, gui::runnable on_hidden) {
    PagesControl* menu = gui->getMenu();
    Panel* panel = new Panel(vec2(500, 200), vec4(8.0f), 8.0f);
    panel->color(vec4(0.0f, 0.0f, 0.0f, 0.5f));
    panel->add(new Label(text));
    panel->add((new Button(langs::get(L"Ok"), vec4(10.f)))->listenAction([=](GUI* gui) {
        if (on_hidden)
            on_hidden();
        menu->back();
    }));
    panel->refresh();
    menu->add("<alert>", panel);
    menu->set("<alert>");
}

void guiutil::confirm(GUI* gui, wstring text, gui::runnable on_confirm,
                      wstring yestext, wstring notext) {
    if (yestext.empty()) yestext = langs::get(L"Yes");
    if (notext.empty()) notext = langs::get(L"No");

    PagesControl* menu = gui->getMenu();
    Panel* panel = new Panel(vec2(600, 200), vec4(8.0f), 8.0f);
    panel->color(vec4(0.0f, 0.0f, 0.0f, 0.5f));
    panel->add(new Label(text));
    Panel* subpanel = new Panel(vec2(600, 53));
    subpanel->color(vec4(0));
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
