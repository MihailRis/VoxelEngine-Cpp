#include "opengl_util.hpp"
#include <GL/glew.h>
#include <string>

bool isGlExtensionSupported(const char *extension) {
    if (!extension || !*extension) {
        return false;
    }

    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for (GLint i = 0; i < numExtensions; ++i) {
        const char *ext = reinterpret_cast<const char *>(glGetStringi(GL_EXTENSIONS, i));
        if (ext && strcmp(ext, extension) == 0) {
            return true;
        }
    }
    return false;
}