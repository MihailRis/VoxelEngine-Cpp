#include "PostEffect.hpp"

#include "Shader.hpp"

PostEffect::Param::Param() : type(Type::FLOAT) {}

PostEffect::Param::Param(Type type) : type(type) {}

PostEffect::PostEffect(std::unique_ptr<Shader> shader)
    : shader(std::move(shader)) {
}

void PostEffect::use() {
    shader->use();
}
