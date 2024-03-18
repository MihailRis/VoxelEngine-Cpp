#ifndef GRAPHICS_CORE_CUBEMAP_H_
#define GRAPHICS_CORE_CUBEMAP_H_

#include "Texture.h"

/// @brief Cubemap texture
class Cubemap : public Texture {
public:
    Cubemap(uint width, uint height, ImageFormat format);

    virtual void bind() override;
    virtual void unbind() override;
};

#endif // GRAPHICS_CORE_CUBEMAP_H_
