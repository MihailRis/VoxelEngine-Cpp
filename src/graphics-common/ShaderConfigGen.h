//
// Created by chelovek on 1/5/24.
//

#ifndef SHADERCONFIG_H
#define SHADERCONFIG_H

#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan_core.h>

#include "../graphics-vk/uniforms/DynamicConstants.h"

template<uint32_t N, size_t T, size_t ...Args>
struct vertex_offset_counter;

template<size_t T, size_t ...Args>
struct vertex_offset_counter<1, T, Args...> {
    static constexpr size_t size = T;
};

template<size_t T, size_t ...Args>
struct vertex_offset_counter<0, T, Args...> {
    static constexpr size_t size = 0;
};

template<uint32_t N, size_t T, size_t ...Args>
struct vertex_offset_counter {
    static constexpr size_t size = T + vertex_offset_counter<N - 1, Args...>::size;
};

template<uint32_t N, size_t ...Args>
inline constexpr size_t vertex_offset_counter_v = vertex_offset_counter<N, Args...>::size;

template<typename T, uint32_t L, VkFormat F>
struct VertexEntry {
    static constexpr size_t size = sizeof(T);
    static constexpr uint32_t location = L;
    static constexpr VkFormat format = F;
};

struct VertexAttrib {
    size_t size;
    uint32_t location;
    VkFormat format;
    size_t offset;
};

template<typename ...Args>
struct VertexConfig {
    static constexpr size_t stride = (Args::size + ...);
    static constexpr std::initializer_list<VkVertexInputAttributeDescription> vertex_attribs = {
        VkVertexInputAttributeDescription{Args::location, 0, Args::format, vertex_offset_counter_v<Args::location, Args::size...>}...
    };
};

template<>
struct VertexConfig<> {
    static constexpr size_t stride = 0;
    static constexpr std::initializer_list<VkVertexInputAttributeDescription> vertex_attribs = { };
};

template<uint32_t B, uint32_t C, VkDescriptorType T, VkShaderStageFlags F>
struct UniformEntry {
    static constexpr uint32_t binding = B;
    static constexpr uint32_t count = C;
    static constexpr VkDescriptorType type = T;
    static constexpr VkShaderStageFlags stages = F;
};

template<typename ...Args>
struct UniformConfig {
    static constexpr std::initializer_list<VkDescriptorSetLayoutBinding> uniform_bindings = {
        VkDescriptorSetLayoutBinding{Args::binding, Args::type, Args::count, Args::stages, nullptr}...
    };
};

template<>
struct UniformConfig<> {
    static constexpr std::initializer_list<VkDescriptorSetLayoutBinding> uniform_bindings = {};
};

template<uint32_t B, uint32_t C, VkShaderStageFlags S>
struct SamplerEntry {
    static constexpr uint32_t binding = B;
    static constexpr uint32_t count = C;
    static constexpr VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    static constexpr VkShaderStageFlags stages = S;
};

template<typename ...Args>
struct SamplerConfig {
    static constexpr std::initializer_list<VkDescriptorSetLayoutBinding> sampler_bindings = {
        VkDescriptorSetLayoutBinding{Args::binding, Args::type, Args::count, Args::stages, nullptr}...
    };
};

template<>
struct SamplerConfig<> {
    static constexpr std::initializer_list<VkDescriptorSetLayoutBinding> sampler_bindings = { };
};

template<typename T, uint32_t O, VkShaderStageFlags F>
struct PushConstantEntry {
    static_assert(sizeof(T) <= 256, "Size push constant grater then 256");
    static constexpr uint32_t size = sizeof(T);
    static constexpr uint32_t offset = O;
    static constexpr VkShaderStageFlags stages = F;
};

struct PushConstantRange {
    uint32_t size;
    uint32_t offset;
    VkShaderStageFlags stages;
};

template<typename ...Args>
struct PushConstantConfig {
    static constexpr std::initializer_list<VkPushConstantRange> ranges = {
        VkPushConstantRange{Args::stages, Args::offset, Args::size}...
    };
};

template<>
struct PushConstantConfig<> {
    static constexpr std::initializer_list<VkPushConstantRange> ranges = {};
};

template<typename Vc = VertexConfig<>, typename Uc = UniformConfig<>, typename Sc = SamplerConfig<>, typename Pc = PushConstantConfig<>>
struct ShaderConfigGen {
    static constexpr size_t vertex_range = Vc::stride;
    static constexpr decltype(Vc::vertex_attribs) vertecies = Vc::vertex_attribs;
    static constexpr decltype(Uc::uniform_bindings) uniforms = Uc::uniform_bindings;
    static constexpr decltype(Sc::sampler_bindings) samplers = Sc::sampler_bindings;
    static constexpr decltype(Pc::ranges) pushConstants = Pc::ranges;
};

using ShaderMainConfig = ShaderConfigGen<
    VertexConfig<
        VertexEntry<glm::vec3, 0, VK_FORMAT_R32G32B32_SFLOAT>,
        VertexEntry<glm::vec2, 1, VK_FORMAT_R32G32_SFLOAT>,
        VertexEntry<float, 2, VK_FORMAT_R32_SFLOAT>
    >,
    UniformConfig<
        UniformEntry<0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT>,
        UniformEntry<1, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT>
    >,
    SamplerConfig<
        SamplerEntry<0, 1, VK_SHADER_STAGE_FRAGMENT_BIT>,
        SamplerEntry<1, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT>
    >,
    PushConstantConfig<
        PushConstantEntry<DynamicConstants, 0, VK_SHADER_STAGE_VERTEX_BIT>
    >
>;

using ShaderUiConfig = ShaderConfigGen<
    VertexConfig<
        VertexEntry<glm::vec2, 0, VK_FORMAT_R32G32_SFLOAT>,
        VertexEntry<glm::vec2, 1, VK_FORMAT_R32G32_SFLOAT>,
        VertexEntry<glm::vec4, 2, VK_FORMAT_R32G32B32A32_SFLOAT>
    >,
    UniformConfig<
        UniformEntry<0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT>
    >,
    SamplerConfig<
        SamplerEntry<0, 1, VK_SHADER_STAGE_FRAGMENT_BIT>
    >
>;

using ShaderLinesConfig = ShaderConfigGen<
    VertexConfig<
        VertexEntry<glm::vec3, 0, VK_FORMAT_R32G32B32_SFLOAT>,
        VertexEntry<glm::vec4, 1, VK_FORMAT_R32G32B32A32_SFLOAT>
    >,
    UniformConfig<
        UniformEntry<0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT>
    >
>;

using ShaderUi3DConfig = ShaderConfigGen<
    VertexConfig<
        VertexEntry<glm::vec3, 0, VK_FORMAT_R32G32B32_SFLOAT>,
        VertexEntry<glm::vec2, 1, VK_FORMAT_R32G32_SFLOAT>,
        VertexEntry<glm::vec4, 2, VK_FORMAT_R32G32B32A32_SFLOAT>
    >,
    UniformConfig<
        UniformEntry<0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT>,
        UniformEntry<1, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT>
    >,
    SamplerConfig<
        SamplerEntry<0, 1, VK_SHADER_STAGE_FRAGMENT_BIT>
    >
>;

using SkyboxBackgroundVertexConfig = VertexConfig<VertexEntry<glm::vec2, 0, VK_FORMAT_R32G32_SFLOAT>>;

using ShaderSkyboxConfig = ShaderConfigGen<
    SkyboxBackgroundVertexConfig,
    UniformConfig<
        UniformEntry<0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT>
    >
>;

using ShaderBackgroundConfig = ShaderConfigGen<
    SkyboxBackgroundVertexConfig,
    UniformConfig<
        UniformEntry<0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT>
    >,
    SamplerConfig<
        SamplerEntry<0, 1, VK_SHADER_STAGE_FRAGMENT_BIT>
    >
>;

#endif //SHADERCONFIG_H
