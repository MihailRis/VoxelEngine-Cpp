#include "ParticlesRenderer.hpp"

#include <set>

#include "assets/Assets.hpp"
#include "assets/assets_util.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/Texture.hpp"
#include "graphics/render/MainBatch.hpp"
#include "window/Camera.hpp"
#include "world/Level.hpp"
#include "voxels/Chunks.hpp"
#include "settings.hpp"

size_t ParticlesRenderer::visibleParticles = 0;
size_t ParticlesRenderer::aliveEmitters = 0;

ParticlesRenderer::ParticlesRenderer(
    const Assets& assets,
    const Level& level,
    const Chunks& chunks,
    const GraphicsSettings* settings
)
    : batch(std::make_unique<MainBatch>(4096)),
      level(level),
      chunks(chunks),
      assets(assets),
      settings(settings) {
}

ParticlesRenderer::~ParticlesRenderer() = default;

static inline void update_particle(
    Particle& particle, float delta, const Chunks& chunks
) {
    const auto& preset = particle.emitter->preset;
    auto& pos = particle.position;
    auto& vel = particle.velocity;

    vel += delta * preset.acceleration;
    if (preset.collision && chunks.isObstacleAt(pos + vel * delta)) {
        vel *= 0.0f;
    }
    pos += vel * delta;
    particle.lifetime -= delta;
}

void ParticlesRenderer::renderParticles(const Camera& camera, float delta) {
    const auto& right = camera.right;
    const auto& up = camera.up;

    bool backlight = settings->backlight.get();

    std::vector<const Texture*> unusedTextures;

    for (auto& [texture, vec] : particles) {
        if (vec.empty()) {
            unusedTextures.push_back(texture);
            continue;
        }
        batch->setTexture(texture);

        visibleParticles += vec.size();

        auto iter = vec.begin();
        while (iter != vec.end()) {
            auto& particle = *iter;
            auto& preset = particle.emitter->preset;

            if (!preset.frames.empty()) {
                float time = preset.lifetime - particle.lifetime;
                int framesCount = preset.frames.size();
                int frameid = time / preset.lifetime * framesCount;
                int frameid2 = glm::min(
                    (time + delta) / preset.lifetime * framesCount,
                    framesCount - 1.0f
                );
                if (frameid2 != frameid) {
                    auto tregion = util::get_texture_region(
                        assets, preset.frames.at(frameid2), ""
                    );
                    if (tregion.texture == texture) {
                        particle.region = tregion.region;
                    }
                }
            }
            update_particle(particle, delta, chunks);

            glm::vec4 light(1, 1, 1, 0);
            if (preset.lighting) {
                light = MainBatch::sampleLight(
                    particle.position, chunks, backlight
                );
                light *= 0.9f + (particle.random % 100) * 0.001f;
            }
            float scale = 1.0f + ((particle.random ^ 2628172) % 1000) *
                                     0.001f * preset.sizeSpread;
            batch->quad(
                particle.position,
                right,
                preset.globalUpVector ? glm::vec3(0, 1, 0) : up,
                preset.size * scale,
                light,
                glm::vec3(1.0f),
                particle.region
            );
            if (particle.lifetime <= 0.0f) {
                iter = vec.erase(iter);
            } else {
                iter++;
            }
        }
    }
    batch->flush();
    for (const auto& texture : unusedTextures) {
        particles.erase(texture);
    }
}

void ParticlesRenderer::render(const Camera& camera, float delta) {
    batch->begin();
    
    aliveEmitters = emitters.size();
    visibleParticles = 0;

    renderParticles(camera, delta);

    auto iter = emitters.begin();
    while (iter != emitters.end()) {
        auto& emitter = *iter->second;
        auto texture = emitter.getTexture();
        const auto& found = particles.find(texture);
        std::vector<Particle>* vec;
        if (found == particles.end()) {
            if (emitter.isDead()) {
                // destruct Emitter only when there is no particles spawned by it
                iter = emitters.erase(iter);
                continue;
            }
            vec = &particles[texture];
        } else {
            vec = &found->second;
        }
        emitter.update(delta, camera.position, *vec);
        iter++;
    }
}

void ParticlesRenderer::gc() {
    std::set<Emitter*> usedEmitters;
    for (const auto& [_, vec] : particles) {
        for (const auto& particle : vec) {
            usedEmitters.insert(particle.emitter);
        }
    }
    auto iter = emitters.begin();
    while (iter != emitters.end()) {
        auto emitter = iter->second.get();
        if (usedEmitters.find(emitter) == usedEmitters.end()) {
            iter = emitters.erase(iter);
        } else {
            iter++;
        }
    }
}

Emitter* ParticlesRenderer::getEmitter(u64id_t id) const {
    const auto& found = emitters.find(id);
    if (found == emitters.end()) {
        return nullptr;
    }
    return found->second.get();
}

u64id_t ParticlesRenderer::add(std::unique_ptr<Emitter> emitter) {
    u64id_t uid = nextEmitter++;
    emitters[uid] = std::move(emitter);
    return uid;
}
