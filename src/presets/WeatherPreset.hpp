#pragma once

#include "interfaces/Serializable.hpp"

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
    } fall {};

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
