#ifndef FRONTEND_GUI_GUI_UTIL_H_
#define FRONTEND_GUI_GUI_UTIL_H_

#include <string>
#include "GUI.h"

namespace gui {
    class Button;
}

namespace guiutil {
    gui::Button* backButton(gui::PagesControl* menu);
    gui::Button* gotoButton(std::wstring text, const std::string& page, gui::PagesControl* menu);
    void alert(gui::GUI* gui, const std::wstring& text, gui::runnable on_hidden=nullptr);
    void confirm(gui::GUI* gui, const std::wstring& text, gui::runnable on_confirm=nullptr,
                 std::wstring yestext=L"", std::wstring notext=L"");
}

#endif // FRONTEND_GUI_GUI_UTIL_H_
