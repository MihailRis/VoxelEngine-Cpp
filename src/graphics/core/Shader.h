#ifndef GRAPHICS_CORE_SHADER_H_
#define GRAPHICS_CORE_SHADER_H_

#include <string>
#include <glm/glm.hpp>
#include "../../typedefs.h"

class GLSLExtension;

class Shader {
    uint id;
public:
    static GLSLExtension* preprocessor;

    Shader(unsigned int id);
    ~Shader();

    void use();
    void uniformMatrix(std::string name, glm::mat4 matrix);
    void uniform1i(std::string name, int x);
    void uniform1f(std::string name, float x);
    void uniform2f(std::string name, float x, float y);
    void uniform2f(std::string name, glm::vec2 xy);
    void uniform2i(std::string name, glm::ivec2 xy);
    void uniform3f(std::string name, float x, float y, float z);
    void uniform3f(std::string name, glm::vec3 xyz);

    static Shader* create(
        std::string vertexFile, 
        std::string fragmentFile,
        std::string vertexSource, 
        std::string fragmentSource
    );
};

#endif // GRAPHICS_SHADER_H_
