#ifndef FRONTEND_SCREENS_H_
#define FRONTEND_SCREENS_H_

#include <memory>
#include "../settings.h"
#include "../util/ObjectsKeeper.hpp"

class Assets;
class Level;
class WorldRenderer;
class Hud;
class Engine;
class Camera;
class Batch2D;
class LevelFrontend;
class LevelController;
class TextureAnimator;

/// @brief Screen is a mainloop state
class Screen : public util::ObjectsKeeper {
protected:
    Engine* engine;
    std::unique_ptr<Batch2D> batch;
public:
    Screen(Engine* engine);
    virtual ~Screen();
    virtual void update(float delta) = 0;
    virtual void draw(float delta) = 0;
    virtual void onEngineShutdown() {};
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
    std::unique_ptr<LevelFrontend> frontend;
    std::unique_ptr<Hud> hud;
    std::unique_ptr<LevelController> controller;
    std::unique_ptr<WorldRenderer> worldRenderer;
    std::unique_ptr<TextureAnimator> animator;

    bool hudVisible = true;
    void updateHotkeys();
public:
    LevelScreen(Engine* engine, Level* level);
    ~LevelScreen();

    void update(float delta) override;
    void draw(float delta) override;

    void onEngineShutdown() override;

    LevelController* getLevelController() const;
};

#endif // FRONTEND_SCREENS_H_
