#include "PostProcessing.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Framebuffer.h"

#include <stdexcept>

PostProcessing::PostProcessing() {
    // Fullscreen quad mesh bulding
    float vertices[] {
        -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, 1.0f, 1.0f, -1.0f
    };
    vattr attrs[] {{2}, {0}};
    quadMesh = std::make_unique<Mesh>(vertices, 6, attrs);
}

PostProcessing::~PostProcessing() {
}

void PostProcessing::use(GfxContext& context) {
    const auto& vp = context.getViewport();
    if (fbo) {
        fbo->resize(vp.getWidth(), vp.getHeight());
    } else {
        fbo = std::make_unique<Framebuffer>(vp.getWidth(), vp.getHeight());
    }
    context.setFramebuffer(fbo.get());
}

void PostProcessing::render(const GfxContext& context, Shader* screenShader) {
    if (fbo == nullptr) {
        throw std::runtime_error("'use(...)' was never called");
    }

    const auto& viewport = context.getViewport();
    screenShader->use();
    screenShader->uniform2i("u_screenSize", viewport.size());
    fbo->getTexture()->bind();
    quadMesh->draw();
}

std::unique_ptr<ImageData> PostProcessing::toImage() {
    return fbo->getTexture()->readData();
}

Framebuffer* PostProcessing::getFramebuffer() const {
    return fbo.get();
}
