#include "WeatherPreset.hpp"

#include "data/dv_util.hpp"

dv::value WeatherPreset::serialize() const {
    auto root = dv::object();
    
    auto froot = dv::object();
    froot["texture"] = fall.texture;
    froot["vspeed"] = fall.vspeed;
    froot["hspeed"] = fall.hspeed;
    froot["scale"] = fall.scale;
    root["fall"] = froot;

    return root;
}

void WeatherPreset::deserialize(const dv::value& src) {
    if (src.has("fall")) {
        const auto& froot = src["fall"];
        froot.at("texture").get(fall.texture);
        froot.at("vspeed").get(fall.vspeed);
        froot.at("hspeed").get(fall.hspeed);
        froot.at("scale").get(fall.scale);
    }
}
