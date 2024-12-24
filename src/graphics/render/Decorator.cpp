#include "Decorator.hpp"

#include "ParticlesRenderer.hpp"
#include "WorldRenderer.hpp"
#include "TextsRenderer.hpp"
#include "TextNote.hpp"
#include "assets/assets_util.hpp"
#include "content/Content.hpp"
#include "voxels/Chunks.hpp"
#include "voxels/Block.hpp"
#include "world/Level.hpp"
#include "window/Camera.hpp"
#include "objects/Player.hpp"
#include "objects/Players.hpp"
#include "logic/LevelController.hpp"
#include "util/stringutil.hpp"
#include "engine.hpp"
#include "files/files.hpp"

namespace fs = std::filesystem;

/// @brief Not greather than 64 for this BIG_PRIME value
inline constexpr int UPDATE_AREA_DIAMETER = 32;
/// @brief Number of blocks in the volume
inline constexpr int UPDATE_BLOCKS =
    UPDATE_AREA_DIAMETER * UPDATE_AREA_DIAMETER * UPDATE_AREA_DIAMETER;
/// @brief Number of update iterations
inline constexpr int ITERATIONS = 512;
/// @brief Big prime number used for pseudo-random 3d array iteration
inline constexpr int BIG_PRIME = 666667;

Decorator::Decorator(
    Engine& engine,
    LevelController& controller,
    WorldRenderer& renderer,
    const Assets& assets,
    Player& player
)
    : engine(engine),
      level(*controller.getLevel()),
      renderer(renderer),
      assets(assets),
      player(player) {
    controller.getBlocksController()->listenBlockInteraction(
    [this](auto player, const auto& pos, const auto& def, BlockInteraction type) {
        if (type == BlockInteraction::placing && def.particles) {
            addParticles(def, pos);
        }
    });
    for (const auto& [id, player] : *level.players) {
        if (id == this->player.getId()) {
            continue;
        }
        playerTexts[id] = renderer.texts->add(std::make_unique<TextNote>(
            util::str2wstr_utf8(player->getName()),
            playerNamePreset,
            player->getPosition()
        ));
    }
    playerNamePreset.deserialize(engine.getResPaths()->readCombinedObject(
        "presets/text3d/player_name.toml"
    ));
}

void Decorator::addParticles(const Block& def, const glm::ivec3& pos) {
    const auto& found = blockEmitters.find(pos);
    if (found == blockEmitters.end()) {
        auto treg = util::get_texture_region(
            assets, def.particles->texture, ""
        );
        blockEmitters[pos] = renderer.particles->add(std::make_unique<Emitter>(
            level,
            glm::vec3{pos.x + 0.5, pos.y + 0.5, pos.z + 0.5},
            *def.particles,
            treg.texture,
            treg.region,
            -1
        ));
    }
}

void Decorator::update(
    float delta, const glm::ivec3& areaStart, const glm::ivec3& areaCenter
) {
    int index = currentIndex;
    currentIndex = (currentIndex + BIG_PRIME) % UPDATE_BLOCKS;

    const auto& chunks = *player.chunks;
    const auto& indices = *level.content->getIndices();

    int lx = index % UPDATE_AREA_DIAMETER;
    int lz = (index / UPDATE_AREA_DIAMETER) % UPDATE_AREA_DIAMETER;
    int ly = (index / UPDATE_AREA_DIAMETER / UPDATE_AREA_DIAMETER);
    
    auto pos = areaStart + glm::ivec3(lx, ly, lz);

    if (auto vox = chunks.get(pos)) {
        const auto& def = indices.blocks.require(vox->id);
        if (def.particles) {
            addParticles(def, pos);
        }
    }
}

void Decorator::update(float delta, const Camera& camera) {
    glm::ivec3 pos = camera.position;
    for (int i = 0; i < ITERATIONS; i++) {
        update(delta, pos - glm::ivec3(UPDATE_AREA_DIAMETER / 2), pos);
    }
    const auto& chunks = *player.chunks;
    const auto& indices = *level.content->getIndices();
    auto iter = blockEmitters.begin();
    while (iter != blockEmitters.end()) {
        auto emitter = renderer.particles->getEmitter(iter->second);
        if (emitter == nullptr) {
            iter = blockEmitters.erase(iter);
            continue;
        }

        bool remove = false;
        if (auto vox = chunks.get(iter->first)) {
            const auto& def = indices.blocks.require(vox->id);
            if (def.particles == nullptr) {
                remove = true;
            }
        } else {
            iter = blockEmitters.erase(iter);
            continue;
        }
        if (util::distance2(iter->first, glm::ivec3(camera.position)) >
            UPDATE_AREA_DIAMETER * UPDATE_AREA_DIAMETER) {
            remove = true;
        }
        if (remove) {
            emitter->stop();
            iter = blockEmitters.erase(iter);
            continue;
        }
        iter++;
    }

    for (const auto& [id, player] : *level.players) {
        if (id == this->player.getId() ||
            playerTexts.find(id) != playerTexts.end()) {
            continue;
        }
        playerTexts[id] = renderer.texts->add(std::make_unique<TextNote>(
            util::str2wstr_utf8(player->getName()),
            playerNamePreset,
            player->getPosition()
        ));
    }

    auto textsIter = playerTexts.begin();
    while (textsIter != playerTexts.end()) {
        auto note = renderer.texts->get(textsIter->second);
        auto player = level.players->get(textsIter->first);
        if (player == nullptr) {
            textsIter = playerTexts.erase(textsIter);
        } else {
            note->setPosition(player->getPosition() + glm::vec3(0, 1, 0));
            ++textsIter;
        }
    }
}
