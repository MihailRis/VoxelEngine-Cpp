#include "Texture.hpp"
#include "GLTexture.hpp"

std::unique_ptr<Texture> Texture::from(const ImageData* image) {
    return GLTexture::from(image);
}
