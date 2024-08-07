#ifndef FRONTEND_MENU_MENU_HPP_
#define FRONTEND_MENU_MENU_HPP_

#include <data/dynamic.hpp>
#include <graphics/ui/elements/Menu.hpp>

#include <string>
#include <vector>
#include <memory>

class Task;
class Engine;

class UiDocument;

namespace menus {
    /// @brief Create development version label at the top-right screen corner
    void create_version_label(Engine* engine);

    gui::page_loader_func create_page_loader(Engine* engine);

    UiDocument* show(
        Engine* engine, 
        const std::string& name,
        std::vector<dynamic::Value> args
    );

    void show_process_panel(Engine* engine, const std::shared_ptr<Task>& task, const std::wstring& text=L"");

    bool call(Engine* engine, runnable func);
}

#endif // FRONTEND_MENU_MENU_HPP_
