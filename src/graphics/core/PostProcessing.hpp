#pragma once

#include <memory>

class Mesh;
class Shader;
class Framebuffer;
class DrawContext;
class ImageData;

/// @brief Framebuffer with blitting with shaders.
/// @attention Current implementation does not support multiple render passes 
/// for multiple effects. Will be implemented in v0.21
class PostProcessing {
    /// @brief Main framebuffer (lasy field)
    std::unique_ptr<Framebuffer> fbo;
    /// @brief Fullscreen quad mesh as the post-processing canvas
    std::unique_ptr<Mesh> quadMesh;
public:
    PostProcessing();
    ~PostProcessing();

    /// @brief Prepare and bind framebuffer
    /// @param context graphics context will be modified
    void use(DrawContext& context);

    /// @brief Render fullscreen quad using the passed shader 
    /// with framebuffer texture bound
    /// @param context graphics context
    /// @param screenShader shader used for fullscreen quad
    /// @throws std::runtime_error if use(...) wasn't called before
    void render(const DrawContext& context, Shader* screenShader);

    /// @brief Make an image from the last rendered frame
    std::unique_ptr<ImageData> toImage();

    Framebuffer* getFramebuffer() const;
};
