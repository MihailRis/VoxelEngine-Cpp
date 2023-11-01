#include "Framebuffer.h"

#include <GL/glew.h>
#include "Texture.h"

Framebuffer::Framebuffer(int width, int height) : width(width), height(height) {
	// glGenFramebuffers(1, &fbo);
	// bind();
	// GLuint tex;
	// glGenTextures(1, &tex);
	// // glBindTexture(GL_TEXTURE_2D, tex);
	// glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	// // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	// glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
    // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
    // texture = new Texture(tex, width, height);
    // glGenRenderbuffers(1, &depth);
    // glBindRenderbuffer(GL_RENDERBUFFER, depth);
    // glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, width, height);
    // // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    // unbind();


    // bind();
    // glGenFramebuffers(1, &fbo);
    // // create a multisampled color attachment texture
    // unsigned int textureColorBufferMultiSampled;
    // glGenTextures(1, &textureColorBufferMultiSampled);
    // glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    // glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
    // glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    // // create a (also multisampled) renderbuffer object for depth and stencil attachments
    // unsigned int rbo;
    // glGenRenderbuffers(1, &rbo);
    // glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
    // glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    // unbind();
}

Framebuffer::~Framebuffer() {
	delete texture;
	glDeleteFramebuffers(1, &fbo);
}

void Framebuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
