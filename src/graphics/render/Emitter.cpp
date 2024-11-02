#include "Emitter.hpp"

#include <glm/gtc/random.hpp>

#include "graphics/core/Texture.hpp"

Emitter::Emitter(
    std::variant<glm::vec3, entityid_t> origin,
    Particle prototype,
    const Texture* texture,
    float spawnInterval,
    int count
)
    : origin(std::move(origin)),
      prototype(std::move(prototype)),
      texture(texture),
      spawnInterval(spawnInterval),
      count(count),
      behaviour() {
    this->prototype.emitter = this;
}

const Texture* Emitter::getTexture() const {
    return texture;
}

void Emitter::update(float delta, std::vector<Particle>& particles) {
    if (count == 0) {
        return;
    }
    glm::vec3 position {};
    if (auto staticPos = std::get_if<glm::vec3>(&origin)) {
        position = *staticPos;
    } else {
        // TODO: implement for entity origin
    }
    timer += delta;
    while (count && timer > spawnInterval) {
        // spawn particle
        Particle particle = prototype;
        particle.emitter = this;
        particle.position = position;
        particle.velocity += glm::ballRand(1.0f) * explosion;
        particles.push_back(std::move(particle));
        timer -= spawnInterval;
        count--;
    }
}

void Emitter::setExplosion(const glm::vec3& magnitude) {
    explosion = magnitude;
}

bool Emitter::isDead() const {
    return count == 0;
}
