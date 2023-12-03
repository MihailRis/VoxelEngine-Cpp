#ifndef FRONTEND_BLOCKS_PREVIEW_H_
#define FRONTEND_BLOCKS_PREVIEW_H_

#include "../typedefs.h"
#include <glm/glm.hpp>

class IShader;
class Atlas;
class Batch3D;
class Block;
class ContentGfxCache;

class BlocksPreview {
    IShader* shader;
    Atlas* atlas;
    Batch3D* batch;
    const ContentGfxCache* const cache;
public:
    BlocksPreview(IShader* shader, Atlas* atlas, const ContentGfxCache* cache);
    ~BlocksPreview();

    void begin();
    void draw(const Block* block, int x, int y, int size, glm::vec4 tint);
};

#endif // FRONTEND_BLOCKS_PREVIEW_H_