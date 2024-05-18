#include "DrawContext.hpp"

#include <GL/glew.h>

#include "Batch2D.hpp"
#include "Framebuffer.hpp"
#include "../../window/Window.hpp"

static void set_blend_mode(BlendMode mode) {
    switch (mode) {
        case BlendMode::normal:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BlendMode::addition:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case BlendMode::inversion:
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
            break;
    }
}


DrawContext::DrawContext(
    const DrawContext* parent, 
    const Viewport& viewport, 
    Batch2D* g2d
) : parent(parent), 
    viewport(viewport), 
    g2d(g2d) 
{}

DrawContext::~DrawContext() {
    if (g2d) {
        g2d->flush();
    }

    while (scissorsCount--) {
        Window::popScissor();
    }

    if (parent == nullptr)
        return;

    if (fbo != parent->fbo) {
        if (fbo) {
            fbo->unbind();
        }
        if (parent->fbo) {
            parent->fbo->bind();
        }
    }

    Window::viewport(
        0, 0,
        parent->viewport.getWidth(), 
        parent->viewport.getHeight()
    );

    if (depthMask != parent->depthMask) {
        glDepthMask(parent->depthMask);
    }
    if (depthTest != parent->depthTest) {
        if (depthTest) glDisable(GL_DEPTH_TEST);
        else glEnable(GL_DEPTH_TEST);
    }
    if (cullFace != parent->cullFace) {
        if (cullFace) glDisable(GL_CULL_FACE);
        else glEnable(GL_CULL_FACE);
    }
    if (blendMode != parent->blendMode) {
        set_blend_mode(parent->blendMode);
    }
}

const Viewport& DrawContext::getViewport() const {
    return viewport;
}

Batch2D* DrawContext::getBatch2D() const {
    return g2d;
}

DrawContext DrawContext::sub() const {
    auto ctx = DrawContext(this, viewport, g2d);
    ctx.depthTest = depthTest;
    ctx.cullFace = cullFace;
    return ctx;
}

void DrawContext::setViewport(const Viewport& viewport) {
    this->viewport = viewport;
    Window::viewport(
        0, 0,
        viewport.getWidth(),
        viewport.getHeight()
    );
}

void DrawContext::setFramebuffer(Framebuffer* fbo) {
    if (this->fbo == fbo)
        return;
    this->fbo = fbo;
    if (fbo) {
        fbo->bind();
    }
}

void DrawContext::setDepthMask(bool flag) {
    if (depthMask == flag)
        return;
    depthMask = flag;
    glDepthMask(GL_FALSE + flag);
}

void DrawContext::setDepthTest(bool flag) {
    if (depthTest == flag)
        return;
    depthTest = flag;
    if (flag) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void DrawContext::setCullFace(bool flag) {
    if (cullFace == flag)
        return;
    cullFace = flag;
    if (flag) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void DrawContext::setBlendMode(BlendMode mode) {
    if (blendMode == mode)
        return;
    blendMode = mode;
    set_blend_mode(mode);
}

void DrawContext::setScissors(glm::vec4 area) {
    Window::pushScissor(area);
    scissorsCount++;
}
