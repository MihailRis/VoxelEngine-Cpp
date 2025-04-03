#include <gtest/gtest.h>

#include "coders/commons.hpp"
#include "coders/GLSLExtension.hpp"

TEST(GLSLExtension, processing) {
    GLSLExtension glsl;
    glsl.addHeader("sum", 
        "// sum function for glsl\n"
        "float sum(float a, float b) {\n"
        "    return a + b;\n"
        "}\n"
    );
    try {
        auto processed = glsl.process("test.glsl",
            "in vec2 v_uv;\n"
            "uniform sampler2D u_screen;\n"
            "\n"
            "#include /* hell\no */  <   sum     >\n"
            "#param float p_intensity\n"
            "\n"
            "vec4 effect() {\n"
            "    vec4 color = texture(u_screen, v_uv);\n"
            "    return mix(color, 1.0 - color, p_intensity);\n"
            "}\n",
        false);
        std::cout << processed << std::endl;
    } catch (const parsing_error& err) {
        std::cerr << err.errorLog() << std::endl;
        throw;
    }
}
