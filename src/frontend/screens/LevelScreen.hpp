#ifndef FRONTEND_SCREENS_LEVEL_SCREEN_HPP_
#define FRONTEND_SCREENS_LEVEL_SCREEN_HPP_

#include "Screen.hpp"

#include <memory>

class Engine;
class LevelFrontend;
class Hud;
class LevelController;
class WorldRenderer;
class TextureAnimator;
class PostProcessing;
class Level;

class LevelScreen : public Screen {
    std::unique_ptr<LevelFrontend> frontend;
    std::unique_ptr<Hud> hud;
    std::unique_ptr<LevelController> controller;
    std::unique_ptr<WorldRenderer> worldRenderer;
    std::unique_ptr<TextureAnimator> animator;
    std::unique_ptr<PostProcessing> postProcessing;

    void saveWorldPreview();

    bool hudVisible = true;
    void updateHotkeys();
    void initializeContent();
public:
    LevelScreen(Engine* engine, std::unique_ptr<Level> level);
    ~LevelScreen();

    void update(float delta) override;
    void draw(float delta) override;

    void onEngineShutdown() override;

    LevelController* getLevelController() const;
};

#endif // FRONTEND_SCREENS_LEVEL_SCREEN_HPP_
