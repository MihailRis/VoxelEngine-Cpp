#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>

class Level;
class Chunks;
class Camera;
class Assets;
struct Block;
class LevelController;
class ParticlesRenderer;

class Decorator {
    const Level& level;
    const Assets& assets;
    ParticlesRenderer& particles;
    std::unordered_map<glm::ivec3, uint64_t> blockEmitters;
    int currentIndex = 0;

    void update(
        float delta, const glm::ivec3& areaStart, const glm::ivec3& areaCenter
    );
    void addParticles(const Block& def, const glm::ivec3& pos);
public:
    Decorator(
        LevelController& level, ParticlesRenderer& particles, const Assets& assets
    );

    void update(float delta, const Camera& camera);
};
