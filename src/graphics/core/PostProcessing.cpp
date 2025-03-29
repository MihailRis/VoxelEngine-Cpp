#include "PostProcessing.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Framebuffer.hpp"
#include "Viewport.hpp"
#include "DrawContext.hpp"

#include <stdexcept>
#include <GL/glew.h>

PostProcessing::PostProcessing() {
    // Fullscreen quad mesh buiding
    PostProcessingVertex meshData[]{
        {{-1.0f, -1.0f}},
        {{-1.0f, 1.0f}},
        {{1.0f, 1.0f}},
        {{-1.0f, -1.0f}},
        {{1.0f, 1.0f}},
        {{1.0f, -1.0f}},
    };

    quadMesh = std::make_unique<Mesh<PostProcessingVertex>>(meshData, 6, PostProcessingVertex::ATTRIBUTES);
}

PostProcessing::~PostProcessing() = default;

void PostProcessing::use(DrawContext& context) {
    const auto& vp = context.getViewport();
    if (fbo) {
        fbo->resize(vp.getWidth(), vp.getHeight());
    } else {
        fbo = std::make_unique<Framebuffer>(vp.getWidth(), vp.getHeight());
    }
    context.setFramebuffer(fbo.get());
}

void PostProcessing::render(const DrawContext& context, Shader* screenShader) {
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
