#include "ParticlesPreset.hpp"

#include "data/dv_util.hpp"

dv::value ParticlesPreset::serialize() const {
    auto root = dv::object();
    root["texture"] = texture;
    root["collision"] = collision;
    root["lighting"] = lighting;
    root["max_distance"] = maxDistance;
    root["spawn_interval"] = spawnInterval;
    root["lifetime"] = lifetime;
    root["lifetime_spread"] = lifetimeSpread;
    root["acceleration"] = dv::to_value(acceleration);
    root["explosion"] = dv::to_value(explosion);
    root["size"] = dv::to_value(size);
    root["random_sub_uv"] = randomSubUV;
    return root;
}

void ParticlesPreset::deserialize(const dv::value& src) {
    src.at("texture").get(texture);
    src.at("collision").get(collision);
    src.at("lighting").get(lighting);
    src.at("max_distance").get(maxDistance);
    src.at("spawn_interval").get(spawnInterval);
    src.at("lifetime").get(lifetime);
    src.at("lifetime_spread").get(lifetimeSpread);
    src.at("random_sub_uv").get(randomSubUV);
    if (src.has("acceleration")) {
        dv::get_vec(src["acceleration"], acceleration);
    }
    if (src.has("size")) {
        dv::get_vec(src["size"], size);
    }
    if (src.has("explosion")) {
        dv::get_vec(src["explosion"], explosion);
    }
}
