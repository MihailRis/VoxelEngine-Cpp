#pragma once

#include "typedefs.hpp"

class Level;
class Assets;
class Chunks;
class Camera;
class MainBatch;
struct GraphicsSettings;

class PrecipitationRenderer {
    std::unique_ptr<MainBatch> batch;
    const Level& level;
    const Chunks& chunks;
    const Assets& assets;
    const GraphicsSettings* settings;
    float timer = 0.0f;

    int getHeightAt(int x, int z);
public:
    PrecipitationRenderer(
        const Assets& assets,
        const Level& level,
        const Chunks& chunks,
        const GraphicsSettings* settings
    );

    ~PrecipitationRenderer();

    void render(const Camera& camera, float delta);
};
