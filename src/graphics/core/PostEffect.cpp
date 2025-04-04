#include "PostEffect.hpp"

#include "Shader.hpp"

PostEffect::Param::Param() : type(Type::FLOAT) {}

PostEffect::Param::Param(Type type, Value defValue)
    : type(type), defValue(std::move(defValue)) {
}

PostEffect::PostEffect(std::unique_ptr<Shader> shader)
    : shader(std::move(shader)) {
}

void PostEffect::use() {
    shader->use();
}
