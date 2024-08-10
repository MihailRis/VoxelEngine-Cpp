#pragma once

#include "GLTexture.hpp"

/// @brief Cubemap texture
class Cubemap : public GLTexture {
public:
    Cubemap(uint width, uint height, ImageFormat format);

    virtual void bind() override;
    virtual void unbind() override;
};
