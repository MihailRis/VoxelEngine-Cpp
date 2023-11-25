//
// Created by chelovek on 11/17/23.
//

#ifndef SURFACE_H
#define SURFACE_H

#include <vulkan/vulkan.h>

class Instance;

class Surface {
    Instance *m_instance = nullptr;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkSurfaceCapabilitiesKHR m_capabilities{};
    VkSurfaceFormatKHR m_format{};
    VkPresentModeKHR m_presentMode{};
public:
    Surface() = default;
    Surface(VkSurfaceKHR surface, Instance *instance);

    VkSurfaceFormatKHR getFormat() const;
    VkSurfaceCapabilitiesKHR getCapabilities() const;
    VkPresentModeKHR getPresentMode() const;

    operator VkSurfaceKHR() const;

    void destroy();
};



#endif //SURFACE_H
