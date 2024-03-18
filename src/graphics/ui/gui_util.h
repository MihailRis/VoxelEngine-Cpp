#ifndef FRONTEND_GUI_GUI_UTIL_H_
#define FRONTEND_GUI_GUI_UTIL_H_

#include <memory>
#include <string>
#include "GUI.h"
#include "../../delegates.h"

namespace gui {
    class Button;
}

namespace guiutil {
    std::shared_ptr<gui::Button> backButton(
        std::shared_ptr<gui::PagesControl> menu
    );

    std::shared_ptr<gui::Button> gotoButton(
        std::wstring text, 
        const std::string& page, 
        std::shared_ptr<gui::PagesControl> menu
    );

    void alert(
        gui::GUI* gui, 
        const std::wstring& text, 
        runnable on_hidden=nullptr
    );

    void confirm(
        gui::GUI* gui, 
        const std::wstring& text, 
        runnable on_confirm=nullptr,
        std::wstring yestext=L"", 
        std::wstring notext=L"");
}

#endif // FRONTEND_GUI_GUI_UTIL_H_
