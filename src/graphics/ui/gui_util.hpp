#pragma once

#include "GUI.hpp"
#include "typedefs.hpp"
#include "delegates.hpp"

#include <memory>
#include <string>

class Engine;

namespace guiutil {
    /// @brief Create element from XML
    /// @param source XML
    std::shared_ptr<gui::UINode> create(const std::string& source, scriptenv env=0);

    void alert(
        const std::shared_ptr<gui::Menu>& menu,
        const std::wstring& text, 
        const runnable& on_hidden=nullptr
    );

    void confirm(
        Engine& engine,
        const std::wstring& text, 
        const runnable& on_confirm=nullptr,
        const runnable& on_deny=nullptr,
        std::wstring yestext=L"", 
        std::wstring notext=L"");

    void confirm_with_memo(
        const std::shared_ptr<gui::Menu>& menu,
        const std::wstring& text, 
        const std::wstring& memo,
        const runnable& on_confirm=nullptr,
        std::wstring yestext=L"", 
        std::wstring notext=L"");
}
