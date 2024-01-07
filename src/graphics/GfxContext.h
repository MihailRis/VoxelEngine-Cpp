#ifndef GRAPHICS_GFX_CONTEXT_H_
#define GRAPHICS_GFX_CONTEXT_H_

#include "../typedefs.h"
#include "Viewport.h"

#include "../graphics-common/graphicsDefenitions.h"

class GfxContext {
    const GfxContext* parent;
    Viewport& viewport;
    Batch2D* const g2d;
    bool depthTest_ = false;
    bool cullFace_ = false;
public:
    GfxContext(const GfxContext* parent, Viewport& viewport, Batch2D* g2d);
    ~GfxContext();
    
    Batch2D* getBatch2D() const;
    const Viewport& getViewport() const;
    GfxContext sub() const;

    void depthTest(bool flag);
    void cullFace(bool flag);
};

#endif // GRAPHICS_GFX_CONTEXT_H_