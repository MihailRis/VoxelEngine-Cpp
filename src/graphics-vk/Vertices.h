//
// Created by chelovek on 11/24/23.
//

#ifndef VERTICES_H
#define VERTICES_H

#include <stdexcept>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include "ShaderType.h"

struct Vertex3D {
    glm::vec3 position;
    glm::vec2 textureCoord;
    float light;

    static VkVertexInputBindingDescription getBinding() {
        VkVertexInputBindingDescription inputBindingDescription{};
        inputBindingDescription.binding = 0;
        inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindingDescription.stride = sizeof(Vertex3D);

        return inputBindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributes() {
        VkVertexInputAttributeDescription posAttrib{};
        posAttrib.binding = 0;
        posAttrib.location = 0;
        posAttrib.format = VK_FORMAT_R32G32B32_SFLOAT;
        posAttrib.offset = offsetof(Vertex3D, position);

        VkVertexInputAttributeDescription texCoordAttrib{};
        texCoordAttrib.binding = 0;
        texCoordAttrib.location = 1;
        texCoordAttrib.format = VK_FORMAT_R32G32_SFLOAT;
        texCoordAttrib.offset = offsetof(Vertex3D, textureCoord);

        VkVertexInputAttributeDescription lightAttrib{};
        lightAttrib.binding = 0;
        lightAttrib.location = 2;
        lightAttrib.format = VK_FORMAT_R32_SFLOAT;
        lightAttrib.offset = offsetof(Vertex3D, light);

        return { posAttrib, texCoordAttrib, lightAttrib };
    }
};

struct Vertex3DUI {
    glm::vec3 position;
    glm::vec2 textureCoord;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBinding() {
        VkVertexInputBindingDescription inputBindingDescription{};
        inputBindingDescription.binding = 0;
        inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindingDescription.stride = sizeof(Vertex3D);

        return inputBindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributes() {
        VkVertexInputAttributeDescription posAttrib{};
        posAttrib.binding = 0;
        posAttrib.location = 0;
        posAttrib.format = VK_FORMAT_R32G32B32_SFLOAT;
        posAttrib.offset = offsetof(Vertex3DUI, position);

        VkVertexInputAttributeDescription texCoordAttrib{};
        texCoordAttrib.binding = 0;
        texCoordAttrib.location = 1;
        texCoordAttrib.format = VK_FORMAT_R32G32_SFLOAT;
        texCoordAttrib.offset = offsetof(Vertex3DUI, textureCoord);

        VkVertexInputAttributeDescription lightAttrib{};
        lightAttrib.binding = 0;
        lightAttrib.location = 2;
        lightAttrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        lightAttrib.offset = offsetof(Vertex3DUI, color);

        return { posAttrib, texCoordAttrib, lightAttrib };
    }
};

struct VertexLine {
    glm::vec3 position;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBinding() {
        VkVertexInputBindingDescription inputBindingDescription{};
        inputBindingDescription.binding = 0;
        inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindingDescription.stride = sizeof(VertexLine);

        return inputBindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributes() {
        VkVertexInputAttributeDescription posAttrib{};
        posAttrib.binding = 0;
        posAttrib.location = 0;
        posAttrib.format = VK_FORMAT_R32G32B32_SFLOAT;
        posAttrib.offset = offsetof(VertexLine, position);

        VkVertexInputAttributeDescription colorAttrib{};
        colorAttrib.binding = 0;
        colorAttrib.location = 1;
        colorAttrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        colorAttrib.offset = offsetof(VertexLine, color);

        return { posAttrib, colorAttrib };
    }
};

struct VertexBackSkyGen {
    glm::vec2 position;

    static VkVertexInputBindingDescription getBinding() {
        VkVertexInputBindingDescription inputBindingDescription{};
        inputBindingDescription.binding = 0;
        inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindingDescription.stride = sizeof(VertexBackSkyGen);
        return inputBindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributes() {
        VkVertexInputAttributeDescription posAttrib{};
        posAttrib.binding = 0;
        posAttrib.location = 0;
        posAttrib.format = VK_FORMAT_R32G32_SFLOAT;
        posAttrib.offset = offsetof(VertexBackSkyGen, position);

        return { posAttrib };
    }
};

struct Vertex2D {
    glm::vec2 position;
    glm::vec2 textureCoord;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBinding() {
        VkVertexInputBindingDescription inputBindingDescription{};
        inputBindingDescription.binding = 0;
        inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindingDescription.stride = sizeof(Vertex2D);

        return inputBindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributes() {
        VkVertexInputAttributeDescription posAttrib{};
        posAttrib.binding = 0;
        posAttrib.location = 0;
        posAttrib.format = VK_FORMAT_R32G32_SFLOAT;
        posAttrib.offset = offsetof(Vertex2D, position);

        VkVertexInputAttributeDescription texCoordAttrib{};
        texCoordAttrib.binding = 0;
        texCoordAttrib.location = 1;
        texCoordAttrib.format = VK_FORMAT_R32G32_SFLOAT;
        texCoordAttrib.offset = offsetof(Vertex2D, textureCoord);

        VkVertexInputAttributeDescription lightAttrib{};
        lightAttrib.binding = 0;
        lightAttrib.location = 2;
        lightAttrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        lightAttrib.offset = offsetof(Vertex2D, color);

        return { posAttrib, texCoordAttrib, lightAttrib };
    }
};

inline VkVertexInputBindingDescription getVertexBindingByType(ShaderType type) {
    switch (type) {
        case ShaderType::MAIN:
            return Vertex3D::getBinding();
        case ShaderType::LINES:
            return VertexLine::getBinding();
        case ShaderType::UI:
            return Vertex2D::getBinding();
        case ShaderType::BACKGROUND:
        case ShaderType::SKYBOX_GEN:
            return VertexBackSkyGen::getBinding();
        case ShaderType::UI3D:
            return Vertex3DUI::getBinding();
        case ShaderType::NONE:
        default:
            throw std::runtime_error("");
    }
}

inline std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptionByType(ShaderType type) {
    switch (type) {
        case ShaderType::NONE:
            break;
        case ShaderType::MAIN:
            return Vertex3D::getAttributes();
        case ShaderType::LINES:
            return VertexLine::getAttributes();
        case ShaderType::UI:
            return Vertex2D::getAttributes();
        case ShaderType::BACKGROUND:
        case ShaderType::SKYBOX_GEN:
            return VertexBackSkyGen::getAttributes();
        case ShaderType::UI3D:
            return Vertex3DUI::getAttributes();
    }
    return {};
}

#endif //VERTICES_H
