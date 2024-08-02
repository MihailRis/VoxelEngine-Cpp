#ifndef GRAPHICS_CORE_GFX_CONTEXT_HPP_
#define GRAPHICS_CORE_GFX_CONTEXT_HPP_

#include "commons.hpp"
#include "Viewport.hpp"
#include "../../typedefs.hpp"

class Batch2D;
class Framebuffer;

class DrawContext {
    const DrawContext* parent;
    Viewport viewport;
    Batch2D* const g2d;
    Flushable* flushable = nullptr;
    Framebuffer* fbo = nullptr;
    bool depthMask = true;
    bool depthTest = false;
    bool cullFace = false;
    BlendMode blendMode = BlendMode::normal;
    int scissorsCount = 0;
    float lineWidth = 1.0f;
public:
    DrawContext(const DrawContext* parent, Viewport viewport, Batch2D* g2d);
    ~DrawContext();
    
    Batch2D* getBatch2D() const;
    
    const Viewport& getViewport() const;
    DrawContext sub(Flushable* flushable=nullptr) const;

    void setViewport(const Viewport& viewport);
    void setFramebuffer(Framebuffer* fbo);
    void setDepthMask(bool flag);
    void setDepthTest(bool flag);
    void setCullFace(bool flag);
    void setBlendMode(BlendMode mode);
    void setScissors(const glm::vec4& area);
    void setLineWidth(float width);
};

#endif // GRAPHICS_CORE_GFX_CONTEXT_HPP_
