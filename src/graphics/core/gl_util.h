#ifndef GRAPHICS_CORE_GL_UTIL_H_
#define GRAPHICS_CORE_GL_UTIL_H_

#include <GL/glew.h>

#include "ImageData.h"

namespace gl {
    inline GLenum to_gl_format(ImageFormat imageFormat) {
        switch (imageFormat) {
            case ImageFormat::rgb888: return GL_RGB;
            case ImageFormat::rgba8888: return GL_RGBA;
            default: 
                return 0;
        }
    }
}

#endif // GRAPHICS_CORE_GL_UTIL_H_
