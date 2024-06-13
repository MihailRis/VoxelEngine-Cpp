#ifndef FRONTEND_GUI_GUI_UTIL_HPP_
#define FRONTEND_GUI_GUI_UTIL_HPP_

#include "GUI.hpp"
#include "../../typedefs.hpp"
#include "../../delegates.hpp"

#include <memory>
#include <string>

namespace guiutil {
    /// @brief Create element from XML
    /// @param source XML
    std::shared_ptr<gui::UINode> create(const std::string& source, scriptenv env=0);

    void alert(
        gui::GUI* gui, 
        const std::wstring& text, 
        const runnable& on_hidden=nullptr
    );

    void confirm(
        gui::GUI* gui, 
        const std::wstring& text, 
        const runnable& on_confirm=nullptr,
        std::wstring yestext=L"", 
        std::wstring notext=L"");
}

#endif // FRONTEND_GUI_GUI_UTIL_HPP_
