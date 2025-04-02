#include "Cubemap.hpp"

#include <GL/glew.h>

#include "engine/ProfilerGpu.hpp"
#include "gl_util.hpp"

Cubemap::Cubemap(uint width, uint height, ImageFormat imageFormat)
    : GLTexture(0, width, height) {
    VOXELENGINE_PROFILE_GPU("Cubemap::Cubemap");
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    uint format = gl::to_glenum(imageFormat);
    for (uint face = 0; face < 6; face++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
            0,
            format,
            width,
            height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            NULL
        );
    }
}

void Cubemap::bind() const {
    VOXELENGINE_PROFILE_GPU("Cubemap::bind");
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void Cubemap::unbind() const {
    VOXELENGINE_PROFILE_GPU("Cubemap::unbind");
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
