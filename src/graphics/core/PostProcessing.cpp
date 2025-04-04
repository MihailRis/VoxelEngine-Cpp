#include "PostProcessing.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Framebuffer.hpp"
#include "DrawContext.hpp"
#include "PostEffect.hpp"
#include "assets/Assets.hpp"

#include <stdexcept>

PostProcessing::PostProcessing() {
    // Fullscreen quad mesh bulding
    float vertices[] {
        -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f, 1.0f, 1.0f, -1.0f
    };
    VertexAttribute attrs[] {{2}, {0}};
    quadMesh = std::make_unique<Mesh>(vertices, 6, attrs);
}

PostProcessing::~PostProcessing() = default;

void PostProcessing::use(DrawContext& context) {
    const auto& vp = context.getViewport();
    if (fbo) {
        fbo->resize(vp.x, vp.y);
        fboSecond->resize(vp.x, vp.y);
    } else {
        fbo = std::make_unique<Framebuffer>(vp.x, vp.y);
        fboSecond = std::make_unique<Framebuffer>(vp.x, vp.y);
    }
    context.setFramebuffer(fbo.get());
}

void PostProcessing::render(
    const DrawContext& context, const Assets& assets, float timer
) {
    if (fbo == nullptr) {
        throw std::runtime_error("'use(...)' was never called");
    }
    int totalPasses = 0;
    for (const auto& effect : effectSlots) {
        totalPasses += (effect != nullptr && effect->isActive());
    }

    if (totalPasses == 0) {
        auto& effect = assets.require<PostEffect>("default");
        effect.use();
        fbo->getTexture()->bind();
        quadMesh->draw();
        return;
    }

    int currentPass = 1;
    for (const auto& effect : effectSlots) {
        if (effect == nullptr || !effect->isActive()) {
            continue;
        }
        auto& shader = effect->use();

        const auto& viewport = context.getViewport();
        shader.uniform1i("u_screen", 0);
        shader.uniform2i("u_screenSize", viewport);
        shader.uniform1f("u_timer", timer);

        fbo->getTexture()->bind();
        if (currentPass < totalPasses) {
            fboSecond->bind();
        }
        quadMesh->draw();
        if (currentPass < totalPasses) {
            fboSecond->unbind();
            std::swap(fbo, fboSecond);
        }
        currentPass++;
    }
}

std::unique_ptr<ImageData> PostProcessing::toImage() {
    return fbo->getTexture()->readData();
}

Framebuffer* PostProcessing::getFramebuffer() const {
    return fbo.get();
}
