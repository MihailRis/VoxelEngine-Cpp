#ifndef SRC_GRAPHICS_FRAMEBUFFER_H_
#define SRC_GRAPHICS_FRAMEBUFFER_H_

#include "../typedefs.h"

#include <memory>

class Texture;

class Framebuffer {
    uint fbo;
    uint depth;
    uint width;
    uint height;
    uint format;
    std::unique_ptr<Texture> texture;
public:
    Framebuffer(uint fbo, uint depth, std::unique_ptr<Texture> texture);
    Framebuffer(uint width, uint height, bool alpha=false);
    ~Framebuffer();

    /// @brief Use framebuffer
    void bind();

    /// @brief Stop using framebuffer
    void unbind();

    /// @brief Update framebuffer texture size
    /// @param width new width
    /// @param height new height
    void resize(uint width, uint height);

    /// @brief Get framebuffer color attachment
    Texture* getTexture() const;

    /// @brief Get framebuffer width
    uint getWidth() const;
    /// @brief Get framebuffer height
    uint getHeight() const;
};

#endif /* SRC_GRAPHICS_FRAMEBUFFER_H_ */
