#include "ParticlesRenderer.hpp"

#include "assets/Assets.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/Texture.hpp"
#include "graphics/render/MainBatch.hpp"
#include "window/Camera.hpp"

ParticlesRenderer::ParticlesRenderer(const Assets& assets)
    : batch(std::make_unique<MainBatch>(1024)) {

    Emitter emitter(glm::vec3(0, 100, 0), Particle {
        glm::vec3(), glm::vec3(), 5.0f, UVRegion(0,0,1,1)
    }, assets.get<Texture>("gui/error"), 0.003f, -1);
    emitters.push_back(std::move(emitter));
}

ParticlesRenderer::~ParticlesRenderer() = default;

void ParticlesRenderer::render(
    const Assets& assets, const Camera& camera, float delta
) {
    const auto& right = camera.right;
    const auto& up = camera.up;

    batch->begin();
    for (auto& [texture, vec] : particles) {
        batch->setTexture(texture);

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
