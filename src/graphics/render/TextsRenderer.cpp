#include "TextsRenderer.hpp"

#include "TextNote.hpp"
#include "maths/util.hpp"
#include "assets/Assets.hpp"
#include "window/Camera.hpp"
#include "maths/FrustumCulling.hpp"
#include "graphics/core/Font.hpp"
#include "graphics/core/Batch3D.hpp"
#include "graphics/core/Shader.hpp"
#include "presets/NotePreset.hpp"

TextsRenderer::TextsRenderer(const Frustum* frustum) : frustum(frustum) {
}

void TextsRenderer::renderText(
    Batch3D& batch,
    const TextNote& note,
    const DrawContext& context,
    const Assets& assets,
    const Camera& camera,
    const EngineSettings& settings,
    bool hudVisible
) {
    const auto& text = note.getText();
    const auto& preset = note.getPreset();
    const auto& pos = note.getPosition();

    if (util::distance2(pos, camera.position) >
        util::sqr(preset.renderDistance / camera.zoom)) {
        return;
    }

    // Projected notes are displayed on the front layer only
    if (preset.displayMode == NoteDisplayMode::PROJECTED) {
        return;
    }
    auto& font = assets.require<Font>("normal");

    glm::vec3 xvec {1, 0, 0};
    glm::vec3 yvec {0, 1, 0};

    int width = font.calcWidth(text, text.length());
    if (preset.displayMode == NoteDisplayMode::Y_FREE_BILLBOARD ||
        preset.displayMode == NoteDisplayMode::XY_FREE_BILLBOARD) {
        xvec = camera.position - pos;
        xvec.y = 0;
        std::swap(xvec.x, xvec.z);
        xvec.z *= -1;
        xvec = glm::normalize(xvec);
        if (preset.displayMode == NoteDisplayMode::XY_FREE_BILLBOARD) {
            yvec = camera.up;
        }
    }

    if (preset.displayMode != NoteDisplayMode::PROJECTED) {
        if (!frustum->isBoxVisible(pos - xvec * (width * 0.5f), 
                                   pos + xvec * (width * 0.5f))) {
            return;
        }
    }

    batch.setColor(preset.color);
    font.draw(
        batch,
        text,
        pos - xvec * (width * 0.5f) * preset.scale,
        xvec * preset.scale,
        yvec * preset.scale
    );
}

void TextsRenderer::renderTexts(
    Batch3D& batch,
    const DrawContext& context,
    const Assets& assets,
    const Camera& camera,
    const EngineSettings& settings,
    bool hudVisible,
    bool frontLayer
) {
    std::vector<TextNote> notes;
    NotePreset preset;
    preset.displayMode = NoteDisplayMode::STATIC_BILLBOARD;
    preset.color = glm::vec4(0, 0, 0, 1);
    preset.scale = 0.005f;

    notes.emplace_back(
        L"Segmentation fault",
        std::move(preset),
        glm::vec3(0.5f, 99.5f, 0.0015f)
    );
    auto& shader = assets.require<Shader>("ui3d");
    
    shader.use();
    shader.uniformMatrix("u_projview", camera.getProjView());
    shader.uniformMatrix("u_apply", glm::mat4(1.0f));
    batch.begin();

    for (const auto& note : notes) {
        renderText(batch, note, context, assets, camera, settings, hudVisible);
    }
    batch.flush();
}
