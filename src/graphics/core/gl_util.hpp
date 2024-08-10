#pragma once

#include "commons.hpp"
#include "ImageData.hpp"

#include <GL/glew.h>

namespace gl {
    inline GLenum to_glenum(ImageFormat imageFormat) {
        switch (imageFormat) {
            case ImageFormat::rgb888: return GL_RGB;
            case ImageFormat::rgba8888: return GL_RGBA;
            default: 
                return 0;
        }
    }

    inline GLenum to_glenum(DrawPrimitive primitive) {
        static const GLenum primitives[]{
            GL_POINTS,
            GL_LINES,
            GL_TRIANGLES
        };
        return primitives[static_cast<int>(primitive)];
    }
}
