#ifndef GRAPHICS_RENDER_BLOCKS_PREVIEW_HPP_
#define GRAPHICS_RENDER_BLOCKS_PREVIEW_HPP_

#include "typedefs.hpp"

#include <glm/glm.hpp>
#include <memory>

class Assets;
class ImageData;
class Atlas;
class Framebuffer;
class Batch3D;
class Block;
class Content;
class Shader;
class ContentGfxCache;

class BlocksPreview {
    static std::unique_ptr<ImageData> draw(
        const ContentGfxCache* cache,
        Shader* shader,
        Framebuffer* framebuffer,
        Batch3D* batch,
        const Block& block, 
        int size
    );
public:
    static std::unique_ptr<Atlas> build(
        const ContentGfxCache* cache,
        Assets* assets, 
        const Content* content
    );
};

#endif // GRAPHICS_RENDER_BLOCKS_PREVIEW_HPP_
