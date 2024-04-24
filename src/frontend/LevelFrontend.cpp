#include "LevelFrontend.h"

#include "../assets/Assets.h"
#include "../audio/audio.h"
#include "../content/Content.h"
#include "../graphics/core/Atlas.hpp"
#include "../graphics/render/BlocksPreview.hpp"
#include "../logic/LevelController.h"
#include "../logic/PlayerController.h"
#include "../voxels/Block.h"
#include "../world/Level.h"
#include "ContentGfxCache.h"

LevelFrontend::LevelFrontend(LevelController* controller, Assets* assets) 
  : level(controller->getLevel()),
    controller(controller),
    assets(assets),
    contentCache(std::make_unique<ContentGfxCache>(level->content, assets)),
    blocksAtlas(BlocksPreview::build(contentCache.get(), assets, level->content)) 
{
    controller->getPlayerController()->listenBlockInteraction(
        [=](Player*, glm::ivec3 pos, const Block* def, BlockInteraction type) {
            auto material = level->content->findBlockMaterial(def->material);
            if (material == nullptr) {
                return;
            }

            if (type == BlockInteraction::step) {
                auto sound = assets->getSound(material->stepsSound);
                audio::play(
                    sound, 
                    glm::vec3(), 
                    true, 
                    0.333f, 
                    1.0f + (rand() % 6 - 3) * 0.05f, 
                    false,
                    audio::PRIORITY_LOW,
                    audio::get_channel_index("regular")
                );
            } else {
                audio::Sound* sound = nullptr;
                switch (type) {
                    case BlockInteraction::placing:
                        sound = assets->getSound(material->placeSound);
                        break;
                    case BlockInteraction::destruction:
                        sound = assets->getSound(material->breakSound);
                        break; 
                    case BlockInteraction::step:
                        break;   
                }
                audio::play(
                    sound, 
                    glm::vec3(pos.x, pos.y, pos.z) + 0.5f, 
                    false, 
                    1.0f,
                    1.0f + (rand() % 6 - 3) * 0.05f, 
                    false,
                    audio::PRIORITY_NORMAL,
                    audio::get_channel_index("regular")
                );
            }
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

LevelController* LevelFrontend::getController() const {
    return controller;
}
