#pragma once

#include <optional>

#include "interfaces/Serializable.hpp"

#include "ParticlesPreset.hpp"

struct WeatherPreset : Serializable {
    struct {
        /// @brief Precipitation texture
        std::string texture;
        /// @brief Fall sound
        std::string noise;
        /// @brief Vertical speed
        float vspeed = 1.0f;
        /// @brief Max horizontal speed
        float hspeed = 0.1f;
        /// @brief UV scaling
        float scale = 0.1f;
        /// @brief Fall opacity interpreted as zero.
        /// @example if 0.8 then opacity range is 0.8-1.0 for 0.0-1.0 intensity
        float minOpacity = 0.8f;
        /// @brief Fall splash
        std::optional<ParticlesPreset> splash;
    } fall {};

    /// @brief Max weather fog opacity
    float fogOpacity = 0.8f;

    /// @brief Weather fog depth multiplier
    float fogDencity = 2.0f;

    /// @brief Weather fog curve
    float fogCurve = 0.5f;

    float clouds = 0.5f;

    /// @brief Weather effects intensity
    float intensity = 1.0f;

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
