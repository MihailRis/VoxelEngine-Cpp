#pragma once

#include <glm/vec3.hpp>

#include "interfaces/Serializable.hpp"

struct ParticlesPreset : public Serializable {
    /// @brief Collision detection
    bool collision = true;
    /// @brief Apply lighting
    bool lighting = true;
    /// @brief Max distance of actually spawning particles.
    float maxDistance = 32.0f;
    /// @brief Particles spawn interval
    float spawnInterval = 0.1f;
    /// @brief Particle life time
    float lifetime = 5.0f;
    /// @brief Initial velocity
    glm::vec3 velocity {};
    /// @brief Velocity acceleration
    glm::vec3 acceleration {0.0f, -16.0f, 0.0f};
    /// @brief Random velocity magnitude applying to spawned particles.
    glm::vec3 explosion {2.0f};
    /// @brief Particle size
    glm::vec3 size {0.1f};
    /// @brief Texture name
    std::string texture = "";

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
