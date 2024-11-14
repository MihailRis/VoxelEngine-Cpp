#pragma once

class DrawContext;
class Camera;
class Assets;
class Batch3D;
class Frustum;
class TextNote;
struct EngineSettings;

class TextsRenderer {
    const Frustum* frustum;

    void renderText(
        Batch3D& batch,
        const TextNote& note,
        const DrawContext& context,
        const Assets& assets,
        const Camera& camera,
        const EngineSettings& settings,
        bool hudVisible
    );
public:
    TextsRenderer(const Frustum* frustum);

    void renderTexts(
        Batch3D& batch,
        const DrawContext& context,
        const Assets& assets,
        const Camera& camera,
        const EngineSettings& settings,
        bool hudVisible,
        bool frontLayer
    );
};
