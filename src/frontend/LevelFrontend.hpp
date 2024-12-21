#pragma once

#include <memory>

class Level;
class Assets;
class Player;
class ContentGfxCache;
class LevelController;
struct EngineSettings;

class LevelFrontend {
    Level& level;
    LevelController* controller;
    const Assets& assets;
    std::unique_ptr<ContentGfxCache> contentCache;
public:
    LevelFrontend(
        Player* currentPlayer,
        LevelController* controller,
        Assets& assets,
        const EngineSettings& settings
    );
    ~LevelFrontend();

    Level& getLevel();
    const Level& getLevel() const;
    const Assets& getAssets() const;
    const ContentGfxCache& getContentGfxCache() const;
    ContentGfxCache& getContentGfxCache();
    LevelController* getController() const;
};
