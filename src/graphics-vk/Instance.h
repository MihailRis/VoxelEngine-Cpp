#ifndef INSTANCE_H
#define INSTANCE_H

#include <vulkan/vulkan.h>
#include "Surface.h"
#include "../typedefs.h"

class Instance {
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugUtilsMessenger = VK_NULL_HANDLE;
    u32 m_apiVersion;
public:
    Instance() = default;
    Instance(VkInstance insatnce, VkPhysicalDevice device, VkDebugUtilsMessengerEXT debugUtilsMessenger, u32 apiVersion);

    operator VkInstance() const;

    VkPhysicalDevice getPhysicalDevice() const;

    Surface createSurface();

    u32 getApiVersion() const { return m_apiVersion; }

    void destroy(VkSurfaceKHR surface);
    void destroy();

    static Instance create();
};



#endif //INSTANCE_H
