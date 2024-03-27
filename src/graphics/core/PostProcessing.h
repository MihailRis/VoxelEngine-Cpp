#ifndef GRAPHICS_CORE_POST_PROCESSING_H_
#define GRAPHICS_CORE_POST_PROCESSING_H_

#include "Viewport.h"
#include "GfxContext.h"

#include <memory>

class Mesh;
class Shader;
class Framebuffer;

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
    void use(GfxContext& context);

    /// @brief Render fullscreen quad using the passed shader 
    /// with framebuffer texture bound
    /// @param context graphics context
    /// @param screenShader shader used for fullscreen quad
    /// @throws std::runtime_error if use(...) wasn't called before
    void render(const GfxContext& context, Shader* screenShader);
};

#endif // GRAPHICS_CORE_POST_PROCESSING_H_
