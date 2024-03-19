#ifndef VOXELS_CHUNKSCONTROLLER_H_
#define VOXELS_CHUNKSCONTROLLER_H_

#include <memory>
#include "../typedefs.h"

class Level;
class Chunk;
class Chunks;
class ChunksStorage;
class Lighting;
class WorldGenerator;

/// @brief ChunksController manages chunks dynamic loading/unloading
class ChunksController {
private:
    Level* level;
    Chunks* chunks;
    ChunksStorage* chunksStorage;
    Lighting* lighting;
    uint padding;
    std::unique_ptr<WorldGenerator> generator;

    /// @brief Process one chunk: load it or calculate lights for it
    bool loadVisible();
    bool buildLights(std::shared_ptr<Chunk> chunk);
    void createChunk(int x, int y);
public:
    ChunksController(Level* level, uint padding);
    ~ChunksController();

    /// @param maxDuration milliseconds reserved for chunks loading
    void update(int64_t maxDuration);
};

#endif /* VOXELS_CHUNKSCONTROLLER_H_ */
