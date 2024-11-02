#include "ParticlesRenderer.hpp"

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
    const Assets& assets, const Level& level, const GraphicsSettings* settings
)
    : batch(std::make_unique<MainBatch>(1024)),
      level(level),
      settings(settings) {
    auto region = util::get_texture_region(assets, "blocks:grass_top", "");
    emitters.push_back(std::make_unique<Emitter>(glm::vec3(0, 80, 0), Particle {
        nullptr, 0, glm::vec3(), glm::vec3(), 5.0f, region.region
    }, region.texture, 0.002f, -1));
}

ParticlesRenderer::~ParticlesRenderer() = default;

static inline void update_particle(
    Particle& particle, float delta, const Chunks& chunks
) {
    const auto& behave = particle.emitter->behaviour;
    auto& pos = particle.position;
    auto& vel = particle.velocity;

    vel += delta * behave.gravity;
    if (behave.collision && chunks.isObstacleAt(pos + vel * delta)) {
        vel *= 0.0f;
    }
    pos += vel * delta;
    particle.lifetime -= delta;
}

void ParticlesRenderer::renderParticles(const Camera& camera, float delta) {
    const auto& right = camera.right;
    const auto& up = camera.up;

    const auto& chunks = *level.chunks;
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

            update_particle(particle, delta, chunks);

            glm::vec4 light(1, 1, 1, 0);
            if (particle.emitter->behaviour.lighting) {
                light = MainBatch::sampleLight(
                    particle.position, chunks, backlight
                );
                light *= 0.8f + (particle.random % 200) * 0.001f;
            }
            batch->quad(
                particle.position,
                right,
                up,
                glm::vec2(0.3f),
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
        auto& emitter = **iter;
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
        emitter.update(delta, *vec);
        iter++;
    }
}
