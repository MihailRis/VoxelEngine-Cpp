#pragma once

#include <memory>

class Level;
class Assets;
class Player;
class ContentGfxCache;
class LevelController;

class LevelFrontend {
    Level* level;
    LevelController* controller;
    Assets* assets;
    std::unique_ptr<ContentGfxCache> contentCache;
public:
    LevelFrontend(Player* currentPlayer, LevelController* controller, Assets* assets);
    ~LevelFrontend();

    Level* getLevel() const;
    Assets* getAssets() const;
    ContentGfxCache* getContentGfxCache() const;
    LevelController* getController() const;
};
