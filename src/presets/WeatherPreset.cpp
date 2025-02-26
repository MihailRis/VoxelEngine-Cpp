#include "WeatherPreset.hpp"

#include "data/dv_util.hpp"

dv::value WeatherPreset::serialize() const {
    auto root = dv::object();
    
    auto froot = dv::object();
    froot["texture"] = fall.texture;
    froot["vspeed"] = fall.vspeed;
    froot["hspeed"] = fall.hspeed;
    froot["scale"] = fall.scale;
    froot["noise"] = fall.noise;
    if (fall.splash) {
        froot["splash"] = fall.splash->serialize();
    }
    root["fall"] = froot;

    root["fog_opacity"] = fogOpacity;
    root["fog_dencity"] = fogDencity;
    root["fog_curve"] = fogCurve;
    root["clouds"] = clouds;

    return root;
}

void WeatherPreset::deserialize(const dv::value& src) {
    if (src.has("fall")) {
        const auto& froot = src["fall"];
        froot.at("texture").get(fall.texture);
        froot.at("vspeed").get(fall.vspeed);
        froot.at("hspeed").get(fall.hspeed);
        froot.at("scale").get(fall.scale);
        froot.at("noise").get(fall.noise);
        
        if (froot.has("splash")) {
            const auto& sroot = froot["splash"];
            fall.splash = ParticlesPreset {};
            fall.splash->deserialize(sroot);
        }
    }
    src.at("fog_opacity").get(fogOpacity);
    src.at("fog_dencity").get(fogDencity);
    src.at("fog_curve").get(fogCurve);
    src.at("clouds").get(clouds);
}
