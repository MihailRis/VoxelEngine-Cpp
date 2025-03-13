#pragma once

#include <string>

#include "presets/WeatherPreset.hpp"

struct Weather : Serializable {
    WeatherPreset a {};
    WeatherPreset b {};
    std::string nameA;
    std::string nameB;
    float t = 1.0f;
    float speed = 0.0f;

    void update(float delta) {
        t += delta * speed;
        t = std::min(t, 1.0f);
        b.intensity = t;
        a.intensity = 1.0f - t;
    }

    void change(WeatherPreset preset, float time, std::string name="") {
        std::swap(a, b);
        std::swap(nameA, nameB);
        b = std::move(preset);
        t = 0.0f;
        speed = 1.0f / std::max(time, 1.e-5f);
        nameB = std::move(name);
        update(0.0f);
    }

    float fogOpacity() const {
        return b.fogOpacity * t + a.fogOpacity * (1.0f - t);
    }

    float fogDencity() const {
        return b.fogDencity * t + a.fogDencity * (1.0f - t);
    }

    float fogCurve() const {
        return b.fogCurve * t + a.fogCurve * (1.0f - t);
    }

    float thunderRate() const {
        return b.thunderRate * t + a.thunderRate * (1.0f - t);
    }

    dv::value serialize() const override {
        return dv::object({
            {"a", a.serialize()},
            {"b", b.serialize()},
            {"name-a", nameA},
            {"name-b", nameB},
            {"t", t},
            {"speed", speed},
        });
    }

    void deserialize(const dv::value& src) override {
        a.deserializeOpt(src.at("a"));
        b.deserializeOpt(src.at("b"));
        src.at("name-a").get(nameA);
        src.at("name-b").get(nameB);
        src.at("t").get(t);
        src.at("speed").get(speed);
    }
};
