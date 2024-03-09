#include "LevelFrontend.h"

#include "BlocksPreview.h"
#include "ContentGfxCache.h"

#include "../audio/audio.h"
#include "../world/Level.h"
#include "../voxels/Block.h"
#include "../assets/Assets.h"
#include "../graphics/Atlas.h"
#include "../content/Content.h"

#include "../logic/LevelController.h"
#include "../logic/PlayerController.h"

LevelFrontend::LevelFrontend(Level* level, Assets* assets) 
  : level(level),
    assets(assets),
    contentCache(std::make_unique<ContentGfxCache>(level->content, assets)),
    blocksAtlas(BlocksPreview::build(contentCache.get(), assets, level->content)) 
{}

void LevelFrontend::observe(LevelController* controller) {
    controller->getPlayerController()->listenBlockInteraction(
        [=](Player*, glm::ivec3 pos, const Block* def, BlockInteraction type) {
            if (type != BlockInteraction::step) {
                return;
            }
            auto material = level->content->findBlockMaterial(def->material);
            if (material == nullptr) {
                return;
            }

            auto sound = assets->getSound(material->stepsSound);
            audio::play(
                sound, 
                glm::vec3(), 
                true, 
                0.333f, 
                1.0f, 
                false,
                audio::PRIORITY_LOW,
                audio::get_channel_index("regular")
            );
        }
    );
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
