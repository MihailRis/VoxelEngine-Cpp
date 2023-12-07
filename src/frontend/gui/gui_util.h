#ifndef FRONTEND_GUI_GUI_UTIL_H_
#define FRONTEND_GUI_GUI_UTIL_H_

#include <string>
#include "GUI.h"

namespace gui {
    class Button;
}

namespace guiutil {
    gui::Button* backButton(gui::PagesControl* menu);
    gui::Button* gotoButton(std::wstring text, std::string page, gui::PagesControl* menu);
    void alert(gui::GUI* gui, std::wstring text, gui::runnable on_hidden=nullptr);
    void confirm(gui::GUI* gui, std::wstring text, gui::runnable on_confirm=nullptr);
}

#endif // FRONTEND_GUI_GUI_UTIL_H_
