#ifndef FRONTEND_SCREENS_MENU_SCREEN_HPP_
#define FRONTEND_SCREENS_MENU_SCREEN_HPP_

#include "Screen.hpp"

#include <memory>

class Camera;
class Engine;

class MenuScreen : public Screen {
    std::unique_ptr<Camera> uicamera;
public:
    MenuScreen(Engine* engine);
    ~MenuScreen();

    void update(float delta) override;
    void draw(float delta) override;
};


#endif // FRONTEND_SCREENS_MENU_SCREEN_HPP_
