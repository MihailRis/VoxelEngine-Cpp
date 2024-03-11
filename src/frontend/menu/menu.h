#ifndef FRONTEND_MENU_MENU_H_
#define FRONTEND_MENU_MENU_H_

#include <string>

class Engine;
class LevelController;

namespace menus {
    // implemented in menu_settings.cpp 
    extern void create_settings_panel(Engine* engine);

    // implemented in menu_pause.cpp
    extern void create_pause_panel(Engine* engine, LevelController* controller);

    /// @brief Load world, convert if required and set to LevelScreen.
    /// @param name world name
    /// @param engine engine instance
    /// @param confirmConvert automatically confirm convert if requested
    void open_world(std::string name, Engine* engine, bool confirmConvert);

    /// @brief Create development version label at the top-right screen corner
    void create_version_label(Engine* engine);
    void create_menus(Engine* engine);
    void refresh_menus(Engine* engine);
}

#endif // FRONTEND_MENU_MENU_H_
