#ifndef FRONTEND_SCREENS_H_
#define FRONTEND_SCREENS_H_

#include <memory>
#include "../settings.h"

class Assets;
class Level;
class WorldRenderer;
class HudRenderer;
class Engine;
class Camera;
class Batch2D;
class LevelFrontend;
class LevelController;

/* Screen is a mainloop state */
class Screen {
protected:
    Engine* engine;
    std::unique_ptr<Batch2D> batch;
public:
    Screen(Engine* engine);
    virtual ~Screen();
    virtual void update(float delta) = 0;
    virtual void draw(float delta) = 0;
};

class MenuScreen : public Screen {
    std::unique_ptr<Camera> uicamera;
public:
    MenuScreen(Engine* engine);
    ~MenuScreen();

    void update(float delta) override;
    void draw(float delta) override;
};

class LevelScreen : public Screen {
    std::unique_ptr<Level> level;
    std::unique_ptr<LevelFrontend> frontend;
    std::unique_ptr<HudRenderer> hud;
    std::unique_ptr<WorldRenderer> worldRenderer;
    std::unique_ptr<LevelController> controller;
    
    bool hudVisible = true;
    void updateHotkeys();
public:
    LevelScreen(Engine* engine, Level* level);
    ~LevelScreen();

    void update(float delta) override;
    void draw(float delta) override;
};

#endif // FRONTEND_SCREENS_H_
