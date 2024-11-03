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
    /// @brief Velocity acceleration
    glm::vec3 acceleration {0.0f, -16.0f, 0.0f};

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
