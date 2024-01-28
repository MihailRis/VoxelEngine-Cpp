#ifndef GRAPHICS_GFX_CONTEXT_H_
#define GRAPHICS_GFX_CONTEXT_H_

#include "../typedefs.h"
#include "Viewport.h"
#include "../window/Window.h"

class Batch2D;

class GfxContext {
    const GfxContext* parent;
    Viewport& viewport;
    Batch2D* const g2d;
    bool depthMask_ = true;
    bool depthTest_ = false;
    bool cullFace_ = false;
    blendmode blendMode_ = blendmode::normal;
public:
    GfxContext(const GfxContext* parent, Viewport& viewport, Batch2D* g2d);
    ~GfxContext();
    
    Batch2D* getBatch2D() const;
    const Viewport& getViewport() const;
    GfxContext sub() const;

    void depthMask(bool flag);
    void depthTest(bool flag);
    void cullFace(bool flag);
    void blendMode(blendmode mode);
};

#endif // GRAPHICS_GFX_CONTEXT_H_