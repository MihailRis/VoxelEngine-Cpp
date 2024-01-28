#include "Texture.h"
#include <GL/glew.h>
#include <stdexcept>
#include <memory>

#include "ImageData.h"

Texture::Texture(uint id, int width, int height) 
    : id(id), width(width), height(height) {
}

Texture::Texture(ubyte* data, int width, int height, uint format) 
    : width(width), height(height) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        format, GL_UNSIGNED_BYTE, (GLvoid *) data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    glDeleteTextures(1, &id);
}

void Texture::bind(){
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::reload(ubyte* data){
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *) data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

ImageData* Texture::readData() {
    std::unique_ptr<ubyte[]> data (new ubyte[width * height * 4]);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
    glBindTexture(GL_TEXTURE_2D, 0);
    return new ImageData(ImageFormat::rgba8888, width, height, data.release());
}

void Texture::setNearestFilter() {
    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture* Texture::from(const ImageData* image) {
    uint width = image->getWidth();
    uint height = image->getHeight();
    uint format;
    const void* data = image->getData();
    switch (image->getFormat())	{
        case ImageFormat::rgb888: format = GL_RGB; break;
        case ImageFormat::rgba8888: format = GL_RGBA; break;
        default:
            throw std::runtime_error("unsupported image data format");
    }
    return new Texture((ubyte*)data, width, height, format);
}
