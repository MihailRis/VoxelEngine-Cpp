#pragma once

#include <vector>
#include <variant>
#include <glm/glm.hpp>

#include "typedefs.hpp"

#include "maths/UVRegion.hpp"

class Emitter;

struct Particle {
    /// @brief Pointer used to access common behaviour.
    /// Emitter must be utilized after all related particles despawn.
    Emitter* emitter;
    /// @brief Global position
    glm::vec3 position;
    /// @brief Linear velocity
    glm::vec3 velocity;
    /// @brief Remaining life time
    float lifetime;
    /// @brief UV region
    UVRegion region;
};

class Texture;

struct ParticleBehaviour {
    bool collision = true;
    glm::vec3 gravity {0.0f, -16.0f, 0.0f};
};

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
    glm::vec3 explosion {8.0f};
public:
    ParticleBehaviour behaviour;

    Emitter(
        std::variant<glm::vec3, entityid_t> origin,
        Particle prototype,
        const Texture* texture,
        float spawnInterval,
        int count
    );

    explicit Emitter(const Emitter&) = delete;

    /// @return Emitter particles texture
    const Texture* getTexture() const;

    /// @brief Update emitter and spawn particles
    /// @param delta delta time
    /// @param particles destination particles vector
    void update(float delta, std::vector<Particle>& particles);    

    /// @brief Set initial random velocity magitude
    void setExplosion(const glm::vec3& magnitude);

    /// @return true if the emitter has spawned all particles
    bool isDead() const;
};
