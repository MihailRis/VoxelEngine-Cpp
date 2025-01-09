#pragma once

#include "Screen.hpp"

#include <memory>

class Camera;
class Engine;

class MenuScreen : public Screen {
    std::unique_ptr<Camera> uicamera;
public:
    MenuScreen(Engine& engine);
    ~MenuScreen();

    void update(float delta) override;
    void draw(float delta) override;
};
