#include "LevelFrontend.h"

#include "../world/Level.h"
#include "../assets/Assets.h"
#include "../graphics/Atlas.h"
#include "BlocksPreview.h"
#include "ContentGfxCache.h"

LevelFrontend::LevelFrontend(Level* level, Assets* assets) 
: level(level),
  assets(assets),
  contentCache(std::make_unique<ContentGfxCache>(level->content, assets)),
  blocksAtlas(BlocksPreview::build(contentCache.get(), assets, level->content)) {
}

LevelFrontend::~LevelFrontend() {
}

Level* LevelFrontend::getLevel() const {
    return level;
}

Assets* LevelFrontend::getAssets() const {
    return assets;
}

ContentGfxCache* LevelFrontend::getContentGfxCache() const {
    return contentCache.get();
}

Atlas* LevelFrontend::getBlocksAtlas() const {
    return blocksAtlas.get();
}
