#ifndef FRONTEND_BLOCKS_PREVIEW_H_
#define FRONTEND_BLOCKS_PREVIEW_H_

#include "../typedefs.h"
#include <glm/glm.hpp>
#include <memory>

class Viewport;
class Shader;
class Atlas;
class Batch3D;
class Block;
class ContentGfxCache;

class BlocksPreview {
    Shader* shader;
    Atlas* atlas;
    std::unique_ptr<Batch3D> batch;
    const ContentGfxCache* const cache;
    const Viewport* viewport;
public:
    BlocksPreview(Shader* shader, Atlas* atlas, const ContentGfxCache* cache);
    ~BlocksPreview();

    void begin(const Viewport* viewport);
    void draw(const Block* block, int x, int y, int size, glm::vec4 tint);
};

#endif // FRONTEND_BLOCKS_PREVIEW_H_