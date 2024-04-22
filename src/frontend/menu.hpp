#ifndef FRONTEND_MENU_MENU_HPP_
#define FRONTEND_MENU_MENU_HPP_

#include <string>
#include <vector>
#include <memory>

class Task;
class Engine;

namespace dynamic {
    class Value;
}

namespace menus {
    /// @brief Create development version label at the top-right screen corner
    void create_version_label(Engine* engine);
    void create_menus(Engine* engine);
    void show(Engine* engine, const std::string& name, std::vector<std::unique_ptr<dynamic::Value>> args);
    void show_process_panel(Engine* engine, std::shared_ptr<Task> task, std::wstring text=L"");
}

#endif // FRONTEND_MENU_MENU_HPP_
