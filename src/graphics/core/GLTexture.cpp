#include "GLTexture.hpp"

#include <GL/glew.h>

#include <memory>
#include <stdexcept>

#include "engine/ProfilerGpu.hpp"
#include "gl_util.hpp"

uint Texture::MAX_RESOLUTION = 1024;  // Window.initialize overrides it

GLTexture::GLTexture(uint id, uint width, uint height)
    : Texture(width, height), id(id) {
}

GLTexture::GLTexture(
    const ubyte* data, uint width, uint height, ImageFormat imageFormat
)
    : Texture(width, height) {
    VOXELENGINE_PROFILE_GPU("GLTexture::GLTexture");

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum format = gl::to_glenum(imageFormat);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        static_cast<const GLvoid*>(data)
    );
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLTexture::~GLTexture() {
    VOXELENGINE_PROFILE_GPU("GLTexture::~GLTexture");

    glDeleteTextures(1, &id);
}

void GLTexture::bind() const {
    VOXELENGINE_PROFILE_GPU("GLTexture::bind");

    glBindTexture(GL_TEXTURE_2D, id);
}

void GLTexture::unbind() const {
    VOXELENGINE_PROFILE_GPU("GLTexture::unbind");

    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::reload(const ImageData& image) {
    width = image.getWidth();
    height = image.getHeight();
    reload(image.getData());
}

void GLTexture::reload(const ubyte* data) {
    VOXELENGINE_PROFILE_GPU("GLTexture::reload");

    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        static_cast<const GLvoid*>(data)
    );
    glBindTexture(GL_TEXTURE_2D, 0);
}

std::unique_ptr<ImageData> GLTexture::readData() {
    VOXELENGINE_PROFILE_GPU("GLTexture::readData");

    auto data = std::make_unique<ubyte[]>(width * height * 4);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
    glBindTexture(GL_TEXTURE_2D, 0);
    return std::make_unique<ImageData>(
        ImageFormat::rgba8888, width, height, std::move(data)
    );
}

void GLTexture::setNearestFilter() {
    VOXELENGINE_PROFILE_GPU("GLTexture::setNearestFilter");

    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::setMipMapping(bool flag, bool pixelated) {
    VOXELENGINE_PROFILE_GPU("GLTexture::setMipMapping");
    bind();
    if (flag) {
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            pixelated ? GL_NEAREST : GL_LINEAR_MIPMAP_NEAREST
        );
    } else {
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            pixelated ? GL_NEAREST : GL_LINEAR
        );
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

std::unique_ptr<GLTexture> GLTexture::from(const ImageData* image) {
    uint width = image->getWidth();
    uint height = image->getHeight();
    void* data = image->getData();
    return std::make_unique<GLTexture>(
        static_cast<ubyte*>(data), width, height, image->getFormat()
    );
}

uint GLTexture::getId() const {
    return id;
}
