//
// Created by chelovek on 11/20/23.
//

#ifndef ISHADER_H
#define ISHADER_H

#include <string>
#include <glm/glm.hpp>

#ifdef USE_VULKAN
#include <vulkan/vulkan_core.h>
struct DynamicConstants;
#endif


class IShader {
public:
    virtual ~IShader() = default;

    virtual void use() = 0;
    virtual void uniformMatrix(std::string name, glm::mat4 matrix) = 0;
    virtual void uniform1i(std::string name, int x) = 0;
    virtual void uniform1f(std::string name, float x) = 0;
    virtual void uniform2f(std::string name, float x, float y) = 0;
    virtual void uniform2f(std::string name, glm::vec2 xy) = 0;
    virtual void uniform3f(std::string name, float x, float y, float z) = 0;
    virtual void uniform3f(std::string name, glm::vec3 xyz) = 0;

#ifdef USE_VULKAN
    virtual void use(VkCommandBuffer commandBuffer, VkExtent2D extent2D) { }
    virtual void pushConstant(const DynamicConstants &constants) { }
#endif
};



#endif //ISHADER_H
