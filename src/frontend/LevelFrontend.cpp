#include "LevelFrontend.h"

#include "../world/Level.h"
#include "../assets/Assets.h"
#include "BlocksPreview.h"
#include "ContentGfxCache.h"

LevelFrontend::LevelFrontend(Level* level, Assets* assets) 
: level(level),
  assets(assets),
  contentCache(std::make_unique<ContentGfxCache>(level->content, assets)),
  blocksPreview(std::make_unique<BlocksPreview>(assets, contentCache.get())) {

}

LevelFrontend::~LevelFrontend() {
}

Level* LevelFrontend::getLevel() const {
    return level;
}

Assets* LevelFrontend::getAssets() const {
    return assets;
}

BlocksPreview* LevelFrontend::getBlocksPreview() const {
    return blocksPreview.get();
}

ContentGfxCache* LevelFrontend::getContentGfxCache() const {
    return contentCache.get();
}
