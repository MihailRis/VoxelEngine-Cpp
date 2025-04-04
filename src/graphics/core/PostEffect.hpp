#pragma once

#include <memory>
#include <string>
#include <variant>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader;

class PostEffect {
public:
    struct Param {
        enum class Type { FLOAT, VEC2, VEC3, VEC4 };
        using Value = std::variant<float, glm::vec2, glm::vec3, glm::vec4>;

        Type type;
        Value defValue;

        Param();
        Param(Type type, Value defValue);
    };

    PostEffect(std::unique_ptr<Shader> shader);

    void use();
private:
    std::unique_ptr<Shader> shader;
    std::unordered_map<std::string, Param> params;
};
