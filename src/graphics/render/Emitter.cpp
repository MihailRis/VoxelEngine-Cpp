#include "Emitter.hpp"

#include <glm/gtc/random.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "window/Camera.hpp"
#include "graphics/core/Texture.hpp"
#include "objects/Entities.hpp"
#include "world/Level.hpp"

Emitter::Emitter(
    const Level& level,
    std::variant<glm::vec3, entityid_t> origin,
    ParticlesPreset preset,
    const Texture* texture,
    const UVRegion& region,
    int count
)
    : level(level),
      origin(std::move(origin)),
      prototype({this, 0, glm::vec3(), preset.velocity, preset.lifetime, region}),
      texture(texture),
      count(count),
      preset(std::move(preset)) {
    this->prototype.emitter = this;
}

const Texture* Emitter::getTexture() const {
    return texture;
}

void Emitter::update(
    float delta,
    const glm::vec3& cameraPosition,
    std::vector<Particle>& particles
) {
    const float spawnInterval = preset.spawnInterval;
    if (count == 0 || (count == -1 && spawnInterval < FLT_EPSILON)) {
        return;
    }
    glm::vec3 position {};
    if (auto staticPos = std::get_if<glm::vec3>(&origin)) {
        position = *staticPos;
    } else if (auto entityId = std::get_if<entityid_t>(&origin)) {
        if (auto entity = level.entities->get(*entityId)) {
            position = entity->getTransform().pos;
        }
    }
    const float maxDistance = preset.maxDistance;
    if (glm::distance2(position, cameraPosition) > maxDistance * maxDistance) {
        if (count > 0) {
            if (spawnInterval < FLT_EPSILON) {
                count = 0;
                return;
            }
            int skipped = timer / spawnInterval;
            count = std::max(0, count - skipped);
            timer -= skipped * spawnInterval;
        }
        return;
    }
    timer += delta;
    while (count && timer > spawnInterval) {
        // spawn particle
        Particle particle = prototype;
        particle.emitter = this;
        particle.random = random.rand32();
        particle.position = position;
        particle.velocity += glm::ballRand(1.0f) * preset.explosion;
        particles.push_back(std::move(particle));
        timer -= spawnInterval;
        if (count > 0) {
            count--;
        }
    }
}

bool Emitter::isDead() const {
    return count == 0;
}
