#ifndef GRAPHICS_CORE_CUBEMAP_HPP_
#define GRAPHICS_CORE_CUBEMAP_HPP_

#include "Texture.hpp"

/// @brief Cubemap texture
class Cubemap : public Texture {
public:
    Cubemap(uint width, uint height, ImageFormat format);

    virtual void bind() override;
    virtual void unbind() override;
};

#endif // GRAPHICS_CORE_CUBEMAP_HPP_
