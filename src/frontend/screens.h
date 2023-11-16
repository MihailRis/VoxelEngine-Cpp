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

namespace gui {
    class UINode;
}


/* Screen is a mainloop state */
class Screen {
protected:
    Engine* engine;
public:
    Screen(Engine* engine) : engine(engine) {};
    virtual ~Screen() {};
    virtual void update(float delta) = 0;
    virtual void draw(float delta) = 0;
};

class MenuScreen : public Screen {
    std::shared_ptr<gui::UINode> panel;
public:
    MenuScreen(Engine* engine);
    ~MenuScreen();

    void update(float delta) override;
    void draw(float delta) override;
};

class LevelScreen : public Screen {
    Level* level;
    WorldRenderer* worldRenderer;
    HudRenderer* hud;
    bool occlusion;
    void updateHotkeys();
public:
    LevelScreen(Engine* engine, Level* level);
    ~LevelScreen();

    void update(float delta) override;
    void draw(float delta) override;
};

#endif // FRONTEND_SCREENS_H_