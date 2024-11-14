#pragma once

class DrawContext;
class Camera;
class Assets;
class Batch3D;
class Frustum;
class TextNote;
struct EngineSettings;

class TextsRenderer {
    Batch3D& batch;
    const Assets& assets;
    const Frustum& frustum;

    void renderText(
        const TextNote& note,
        const DrawContext& context,
        const Camera& camera,
        const EngineSettings& settings,
        bool hudVisible
    );
public:
    TextsRenderer(Batch3D& batch, const Assets& assets, const Frustum& frustum);

    void renderTexts(
        const DrawContext& context,
        const Camera& camera,
        const EngineSettings& settings,
        bool hudVisible,
        bool frontLayer
    );
};
