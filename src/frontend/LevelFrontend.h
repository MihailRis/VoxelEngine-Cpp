#ifndef FRONTEND_LEVEL_FRONTEND_H_
#define FRONTEND_LEVEL_FRONTEND_H_

#include <memory>

class Atlas;
class Level;
class Assets;
class BlocksPreview;
class ContentGfxCache;

class LevelFrontend {
    Level* level;
    Assets* assets;
    std::unique_ptr<ContentGfxCache> contentCache;
    std::unique_ptr<Atlas> blocksAtlas;
public:
    LevelFrontend(Level* level, Assets* assets);
    ~LevelFrontend();

    Level* getLevel() const;
    Assets* getAssets() const;
    ContentGfxCache* getContentGfxCache() const;
    Atlas* getBlocksAtlas() const;
};


#endif // FRONTEND_LEVEL_FRONTEND_H_
