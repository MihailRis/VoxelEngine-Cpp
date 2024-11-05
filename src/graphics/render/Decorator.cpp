#include "Decorator.hpp"

#include "ParticlesRenderer.hpp"
#include "assets/assets_util.hpp"
#include "content/Content.hpp"
#include "voxels/Chunks.hpp"
#include "voxels/Block.hpp"
#include "world/Level.hpp"
#include "window/Camera.hpp"
#include "logic/LevelController.hpp"

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
    LevelController& controller, ParticlesRenderer& particles, const Assets& assets
)
    : level(*controller.getLevel()), particles(particles), assets(assets) {
    controller.getBlocksController()->listenBlockInteraction(
    [this](auto player, const auto& pos, const auto& def, BlockInteraction type) {
        if (type == BlockInteraction::placing && def.particles) {
            addParticles(def, pos);
        }
    });
}

void Decorator::addParticles(const Block& def, const glm::ivec3& pos) {
    const auto& found = blockEmitters.find(pos);
    if (found == blockEmitters.end()) {
        auto treg = util::get_texture_region(
            assets, def.particles->texture, ""
        );
        blockEmitters[pos] = particles.add(std::make_unique<Emitter>(
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

    const auto& chunks = *level.chunks;
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
    pos -= glm::ivec3(UPDATE_AREA_DIAMETER / 2);
    for (int i = 0; i < ITERATIONS; i++) {
        update(delta, pos, camera.position);
    }
    const auto& chunks = *level.chunks;
    const auto& indices = *level.content->getIndices();
    auto iter = blockEmitters.begin();
    while (iter != blockEmitters.end()) {
        auto emitter = particles.getEmitter(iter->second);
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
}
