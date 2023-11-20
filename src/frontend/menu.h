#ifndef FRONTEND_MENU_H_
#define FRONTEND_MENU_H_

class Engine;

namespace gui {
    class Panel;
    class PagesControl;
}

gui::Panel* create_main_menu_panel(Engine* engine, gui::PagesControl* menu);
gui::Panel* create_new_world_panel(Engine* engine, gui::PagesControl* menu);
gui::Panel* create_controls_panel(Engine* engine, gui::PagesControl* menu);
gui::Panel* create_settings_panel(Engine* engine, gui::PagesControl* menu);
gui::Panel* create_pause_panel(Engine* engine, gui::PagesControl* menu);

#endif // FRONTEND_MENU_H_