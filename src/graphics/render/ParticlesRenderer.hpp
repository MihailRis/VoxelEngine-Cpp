#pragma once

#include <vector>
#include <unordered_map>

#include "Emitter.hpp"

class Texture;
class Assets;
class Camera;
class MainBatch;

class ParticlesRenderer {
    std::unordered_map<const Texture*, std::vector<Particle>> particles;
    std::vector<Emitter> emitters;
    std::unique_ptr<MainBatch> batch;
public:
    ParticlesRenderer(const Assets& assets);
    ~ParticlesRenderer();

    void render(const Assets& assets, const Camera& camera, float delta);

    static size_t visibleParticles;
    static size_t aliveEmitters;
};
