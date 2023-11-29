//
// Created by chelovek on 11/17/23.
//

#include "Surface.h"

#include <vector>

#include "Instance.h"
#include "../typedefs.h"

inline VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
    for (const auto &format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return  format;
    }

    return formats[0];
}

inline VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> &presentModes) {
    for (const auto &presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return presentMode;
    }

    return presentModes[presentModes.size() - 1];
}

Surface::Surface(VkSurfaceKHR surface, Instance *instance)
    : m_instance(instance),
      m_surface(surface) {

    const auto physicalDevice = instance->getPhysicalDevice();
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &m_capabilities);

    u32 surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &surfaceFormatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats{surfaceFormatCount};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &surfaceFormatCount, formats.data());

    u32 presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes{presentModeCount};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, presentModes.data());

    m_format = chooseFormat(formats);
    m_presentMode = choosePresentMode(presentModes);
}

VkSurfaceFormatKHR Surface::getFormat() const {
    return m_format;
}

VkSurfaceCapabilitiesKHR Surface::getCapabilities() const {
    return m_capabilities;
}

VkPresentModeKHR Surface::getPresentMode() const {
    return m_presentMode;
}

Surface::operator VkSurfaceKHR() const {
    return m_surface;
}


void Surface::destroy() {
    m_instance->destroy(m_surface);
}

