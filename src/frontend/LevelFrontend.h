#ifndef FRONTEND_LEVEL_FRONTEND_H_
#define FRONTEND_LEVEL_FRONTEND_H_

#include <memory>

class Atlas;
class Level;
class Assets;
class BlocksPreview;
class ContentGfxCache;
class LevelController;

class LevelFrontend {
    Level* level;
    LevelController* controller;
    Assets* assets;
    std::unique_ptr<ContentGfxCache> contentCache;
    std::unique_ptr<Atlas> blocksAtlas;
public:
    LevelFrontend(LevelController* controller, Assets* assets);
    ~LevelFrontend();

    Level* getLevel() const;
    Assets* getAssets() const;
    ContentGfxCache* getContentGfxCache() const;
    Atlas* getBlocksAtlas() const;

    LevelController* getController() const;
};

#endif // FRONTEND_LEVEL_FRONTEND_H_
