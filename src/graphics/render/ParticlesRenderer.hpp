#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "Emitter.hpp"

class Texture;
class Assets;
class Camera;
class MainBatch;

class ParticlesRenderer {
    std::unordered_map<const Texture*, std::vector<Particle>> particles;
    std::vector<std::unique_ptr<Emitter>> emitters;
    std::unique_ptr<MainBatch> batch;

    void renderParticles(const Camera& camera, float delta);
public:
    ParticlesRenderer(const Assets& assets);
    ~ParticlesRenderer();

    void render(const Camera& camera, float delta);

    static size_t visibleParticles;
    static size_t aliveEmitters;
};
