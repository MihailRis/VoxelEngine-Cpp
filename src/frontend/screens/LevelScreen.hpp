#pragma once

#include "Screen.hpp"

#include <memory>

class Engine;
class LevelFrontend;
class Hud;
class LevelController;
class PlayerController;
class WorldRenderer;
class TextureAnimator;
class PostProcessing;
class ContentPackRuntime;
class Decorator;
class Level;

class LevelScreen : public Screen {
    std::unique_ptr<LevelFrontend> frontend;
    std::unique_ptr<LevelController> controller;
    std::unique_ptr<PlayerController> playerController;
    std::unique_ptr<WorldRenderer> worldRenderer;
    std::unique_ptr<TextureAnimator> animator;
    std::unique_ptr<PostProcessing> postProcessing;
    std::unique_ptr<Decorator> decorator;
    std::unique_ptr<Hud> hud;

    void saveWorldPreview();

    bool hudVisible = true;
    bool debug = false;
    void updateHotkeys();
    void initializeContent();
    void initializePack(ContentPackRuntime* pack);
public:
    LevelScreen(Engine& engine, std::unique_ptr<Level> level);
    ~LevelScreen();

    void update(float delta) override;
    void draw(float delta) override;

    void onEngineShutdown() override;

    LevelController* getLevelController() const;
};
