#pragma once

#include <memory>

#include "typedefs.hpp"

class Level;
class Chunk;
class Chunks;
class Lighting;
class WorldGenerator;

/// @brief ChunksController manages chunks dynamic loading/unloading
class ChunksController {
private:
    Level& level;
    Chunks& chunks;
    uint padding;
    std::unique_ptr<WorldGenerator> generator;

    /// @brief Process one chunk: load it or calculate lights for it
    bool loadVisible();
    bool buildLights(const std::shared_ptr<Chunk>& chunk);
    void createChunk(int x, int y);
public:
    std::unique_ptr<Lighting> lighting;

    ChunksController(Level& level, uint padding);
    ~ChunksController();

    /// @param maxDuration milliseconds reserved for chunks loading
    void update(
        int64_t maxDuration,
        int loadDistance,
        int centerX,
        int centerY);

    const WorldGenerator* getGenerator() const {
        return generator.get();
    }
};
