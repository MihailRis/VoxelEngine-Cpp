#ifndef FRONTEND_LEVEL_FRONTEND_H_
#define FRONTEND_LEVEL_FRONTEND_H_

#include <memory>

class Level;
class Assets;
class BlocksPreview;
class ContentGfxCache;

class LevelFrontend {
    Level* level;
    Assets* assets;
    std::unique_ptr<ContentGfxCache> contentCache;
    std::unique_ptr<BlocksPreview> blocksPreview;
public:
    LevelFrontend(Level* level, Assets* assets);
    ~LevelFrontend();

    Level* getLevel() const;
    Assets* getAssets() const;
    BlocksPreview* getBlocksPreview() const;
    ContentGfxCache* getContentGfxCache() const;
};


#endif // FRONTEND_LEVEL_FRONTEND_H_
