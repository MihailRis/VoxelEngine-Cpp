#include "Framebuffer.h"

#include <GL/glew.h>
#include "Texture.h"

Framebuffer::Framebuffer(uint fbo, uint depth, std::unique_ptr<Texture> texture)
  : fbo(fbo), depth(depth), texture(std::move(texture)) 
{
    if (texture) {
        width = texture->getWidth();
        height = texture->getHeight();
    } else {
        width = 0;
        height = 0;
    }
} 

Framebuffer::Framebuffer(uint width, uint height, bool alpha) 
  : width(width), height(height)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Setup color attachment (texture)
    GLuint tex;
    format = alpha ? GL_RGBA : GL_RGB;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    texture = std::make_unique<Texture>(tex, width, height);
    
    // Setup depth attachment
    glGenRenderbuffers(1, &depth);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &depth);
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(uint width, uint height) {
    if (this->width == width && this->height == height) {
        return;
    }
    GLuint texid = texture->getId();
    texture->bind();
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    texture->unbind(); 
}

Texture* Framebuffer::getTexture() const {
    return texture.get();
}

uint Framebuffer::getWidth() const {
    return width;
}

uint Framebuffer::getHeight() const {
    return height;
}
