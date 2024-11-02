#pragma once

#include <vector>
#include <variant>
#include <glm/glm.hpp>

#include "typedefs.hpp"

#include "maths/UVRegion.hpp"

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    UVRegion region;
};

class Texture;

class Emitter {
    /// @brief Static position or entity
    std::variant<glm::vec3, entityid_t> origin;
    /// @brief Particle prototype
    Particle prototype;
    /// @brief Particle
    const Texture* texture;
    /// @brief Particles spawn interval
    float spawnInterval;
    /// @brief Number of particles should be spawned before emitter deactivation.
    /// -1 is infinite
    int count;
    /// @brief Spawn timer used to determine number of particles 
    /// to spawn on update. May be innacurate.
    float timer = 0.0f;
    /// @brief Random velocity magnitude applying to spawned particles
    glm::vec3 explosion {1.0f};
public:
    Emitter(
        std::variant<glm::vec3, entityid_t> origin,
        Particle prototype,
        const Texture* texture,
        float spawnInterval,
        int count
    );

    const Texture* getTexture() const;

    /// @brief Update emitter and spawn particles
    /// @param delta delta time
    /// @param particles destination particles vector
    void update(float delta, std::vector<Particle>& particles);    

    void setExplosion(const glm::vec3& magnitude);

    bool isDead() const;
};
