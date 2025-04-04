#include "PostEffect.hpp"

#include "Shader.hpp"

PostEffect::Param::Param() : type(Type::FLOAT) {}

PostEffect::Param::Param(Type type, Value defValue)
    : type(type), defValue(std::move(defValue)) {
}

PostEffect::PostEffect(
    std::unique_ptr<Shader> shader,
    std::unordered_map<std::string, Param> params
)
    : shader(std::move(shader)), params(std::move(params)) {
}

Shader& PostEffect::use() {
    shader->use();
    shader->uniform1f("u_intensity", intensity);
    return *shader;
}

void PostEffect::setIntensity(float value) {
    intensity = value;
}
