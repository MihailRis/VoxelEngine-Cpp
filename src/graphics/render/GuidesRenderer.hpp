#pragma once

class LineBatch;
class DrawContext;
class Camera;
class Shader;

class GuidesRenderer {
public:
    void drawBorders(
        LineBatch& batch, int sx, int sy, int sz, int ex, int ey, int ez
    );
    void drawCoordSystem(
        LineBatch& batch, const DrawContext& pctx, float length
    );

    /// @brief Render all debug lines (chunks borders, coord system guides)
    /// @param context graphics context
    /// @param camera active camera
    /// @param linesShader shader used
    void renderDebugLines(
        const DrawContext& context,
        const Camera& camera,
        LineBatch& batch,
        Shader& linesShader,
        bool showChunkBorders
    );
};
