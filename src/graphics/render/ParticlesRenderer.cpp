#include "ParticlesRenderer.hpp"

#include "assets/Assets.hpp"
#include "assets/assets_util.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/Texture.hpp"
#include "graphics/render/MainBatch.hpp"
#include "window/Camera.hpp"

size_t ParticlesRenderer::visibleParticles = 0;
size_t ParticlesRenderer::aliveEmitters = 0;

ParticlesRenderer::ParticlesRenderer(const Assets& assets)
    : batch(std::make_unique<MainBatch>(1024)) {

    auto region = util::get_texture_region(assets, "blocks:grass_side", "");
    Emitter emitter(glm::vec3(0, 100, 0), Particle {
        glm::vec3(), glm::vec3(), 5.0f, region.region
    },region.texture, 0.001f, -1);
    emitters.push_back(std::move(emitter));
}

ParticlesRenderer::~ParticlesRenderer() = default;

void ParticlesRenderer::render(
    const Assets& assets, const Camera& camera, float delta
) {
    const auto& right = camera.right;
    const auto& up = camera.up;

    batch->begin();
    
    aliveEmitters = emitters.size();
    visibleParticles = 0;
    for (auto& [texture, vec] : particles) {
        batch->setTexture(texture);

        visibleParticles += vec.size();

        auto iter = vec.begin();
        while (iter != vec.end()) {
            auto& particle = *iter;

            particle.position += particle.velocity * delta;

            batch->quad(
                particle.position,
                right,
                up,
                glm::vec2(0.3f),
                glm::vec4(1),
                glm::vec3(1.0f),
                particle.region
            );

            particle.lifetime -= delta;
            if (particle.lifetime <= 0.0f) {
                iter = vec.erase(iter);
            } else {
                iter++;
            }
        }
    }
    batch->flush();
    
    auto iter = emitters.begin();
    while (iter != emitters.end()) {
        auto& emitter = *iter;
        auto texture = emitter.getTexture();
        const auto& found = particles.find(texture);
        std::vector<Particle>* vec;
        if (found == particles.end()) {
            vec = &particles[texture];
        } else {
            vec = &found->second;
        }
        emitter.update(delta, *vec);

        if (emitter.isDead()) {
            iter = emitters.erase(iter);
        } else {
            iter++;
        }
    }
}
