#ifndef FRONTEND_SCREENS_H_
#define FRONTEND_SCREENS_H_

#include <memory>
#include "../settings.h"
#include "../graphics-common/graphicsDefenitions.h"

namespace vulkan {
    class WorldRenderer;
}

class Assets;
class Level;
class WorldRenderer;
class HudRenderer;
class Engine;
class Camera;
class ContentGfxCache;
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
    Camera* uicamera;
public:
    MenuScreen(Engine* engine);
    ~MenuScreen();

    void update(float delta) override;
    void draw(float delta) override;
};

class LevelScreen : public Screen {
    Level* level;
    vulkan::WorldRenderer* worldRenderer;
    LevelController* controller;
    HudRenderer* hud;
    ContentGfxCache* cache;

    bool hudVisible = true;
    void updateHotkeys();
public:
    LevelScreen(Engine* engine, Level* level);
    ~LevelScreen();

    void update(float delta) override;
    void draw(float delta) override;
};

#endif // FRONTEND_SCREENS_H_