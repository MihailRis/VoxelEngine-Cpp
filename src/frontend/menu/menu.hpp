#ifndef FRONTEND_MENU_MENU_HPP_
#define FRONTEND_MENU_MENU_HPP_

class Engine;

namespace menus {
    /// @brief Create development version label at the top-right screen corner
    void create_version_label(Engine* engine);
    void create_menus(Engine* engine);
}

#endif // FRONTEND_MENU_MENU_HPP_
