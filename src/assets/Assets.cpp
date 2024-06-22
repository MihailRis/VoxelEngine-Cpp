#include "Assets.hpp"

Assets::~Assets() {
}

const std::vector<TextureAnimation>& Assets::getAnimations() {
    return animations;
}

void Assets::store(const TextureAnimation& animation) {
    animations.emplace_back(animation);
}
