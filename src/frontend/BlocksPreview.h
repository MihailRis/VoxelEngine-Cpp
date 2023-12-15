#ifndef FRONTEND_BLOCKS_PREVIEW_H_
#define FRONTEND_BLOCKS_PREVIEW_H_

#include "../typedefs.h"
#include <glm/glm.hpp>

namespace vulkan {
    class Batch3D;
}

class Viewport;
class IShader;
class Atlas;
class Batch3D;
class Block;
class ContentGfxCache;

class BlocksPreview {
    IShader* shader;
    Atlas* atlas;
    vulkan::Batch3D* batch;
    const ContentGfxCache* const cache;
    const Viewport* viewport;
public:
    BlocksPreview(IShader* shader, Atlas* atlas, const ContentGfxCache* cache);
    ~BlocksPreview();

    void begin(const Viewport* viewport);
    void draw(const Block* block, int x, int y, int size, glm::vec4 tint);
};

#endif // FRONTEND_BLOCKS_PREVIEW_H_