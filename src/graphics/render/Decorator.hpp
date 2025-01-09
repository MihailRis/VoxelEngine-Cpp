#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>

#include "typedefs.hpp"
#include "presets/NotePreset.hpp"

class Level;
class Chunks;
class Camera;
class Assets;
class Player;
class Block;
class Engine;
class LevelController;
class WorldRenderer;

class Decorator {
    Engine& engine;
    const Level& level;
    const Assets& assets;
    Player& player;
    WorldRenderer& renderer;
    std::unordered_map<glm::ivec3, uint64_t> blockEmitters;
    std::unordered_map<int64_t, u64id_t> playerTexts;
    int currentIndex = 0;
    NotePreset playerNamePreset {};

    void update(
        float delta, const glm::ivec3& areaStart, const glm::ivec3& areaCenter
    );
    void addParticles(const Block& def, const glm::ivec3& pos);
public:
    Decorator(
        Engine& engine,
        LevelController& level,
        WorldRenderer& renderer,
        const Assets& assets,
        Player& player
    );

    void update(float delta, const Camera& camera);
};
