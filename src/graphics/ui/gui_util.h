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
        std::shared_ptr<gui::Menu> menu
    );

    std::shared_ptr<gui::Button> gotoButton(
        std::wstring text, 
        const std::string& page, 
        std::shared_ptr<gui::Menu> menu
    );

    /// @brief Create element from XML
    /// @param source XML
    std::shared_ptr<gui::UINode> create(const std::string& source, int env=0);

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
