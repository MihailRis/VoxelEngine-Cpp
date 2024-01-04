//
// Created by chelovek on 11/16/23.
//

#include "Instance.h"
#include <map>

#include "Inistializers.h"
#include "Tools.h"
#include "VulkanDefenitions.h"
#include "../window/Window.h"
#include "../engine.h"

inline u32 ratePhysicalDevice(VkPhysicalDevice physicalDevice) {
    u32 score = 0;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;

    score += properties.limits.maxImageDimension2D;
    return score;
}

inline VkPhysicalDevice pickPhysicalDevice(VkInstance instance) {
    u32 count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);

    if (count == 0) {
        throw initialize_error("[Vulkan] Error while pick physical device");
    }

    std::vector<VkPhysicalDevice> physicalDevices{count};
    vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());

    std::multimap<u32, VkPhysicalDevice> candidates;

    for (const auto &physicalDevice : physicalDevices) {
        candidates.insert(std::make_pair(ratePhysicalDevice(physicalDevice), physicalDevice));
    }

    if (candidates.rbegin()->first > 0)
        return candidates.rbegin()->second;

    return VK_NULL_HANDLE;
}

Instance::Instance(VkInstance insatnce, VkPhysicalDevice device, VkDebugUtilsMessengerEXT debugUtilsMessenger, u32 apiVersion)
    : m_instance(insatnce),
      m_physicalDevice(device),
      m_debugUtilsMessenger(debugUtilsMessenger),
      m_apiVersion(apiVersion)
{ }

Instance::operator VkInstance() const {
    return m_instance;
}

VkPhysicalDevice Instance::getPhysicalDevice() const {
    return m_physicalDevice;
}

Surface Instance::createSurface() {
    return { Window::createWindowSurface(m_instance), this };
}

void Instance::destroy(VkSurfaceKHR surface) {
    vkDestroySurfaceKHR(m_instance, surface, nullptr);
}

void Instance::destroy() {
    if constexpr (IS_DEBUG) {
        tools::destroyDebugUtilsMessneger(m_instance, m_debugUtilsMessenger, nullptr);
    }

    vkDestroyInstance(m_instance, nullptr);
}

Instance Instance::create() {
    auto extensions = Window::getRequiredExtensions();
    std::vector<const char *> layers;

    if constexpr (IS_DEBUG) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Voxel Engine";
    appInfo.applicationVersion = ENGINE_VERSION_VALUE;
    appInfo.pEngineName = "Voxel Engine";
    appInfo.engineVersion = ENGINE_VERSION_VALUE;
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();

    if constexpr (IS_DEBUG) {
        auto debugInfo = initializers::debugUtilsMessengerCreateInfoExt();
        createInfo.pNext = &debugInfo;
    }

    VkInstance instance = VK_NULL_HANDLE;
    CHECK_VK_FUNCTION(vkCreateInstance(&createInfo, nullptr, &instance));

    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;

    if constexpr (IS_DEBUG) {
        const auto debugInfo = initializers::debugUtilsMessengerCreateInfoExt();
        tools::createDebugUtilsMessenger(instance, &debugInfo, nullptr, &debugUtilsMessenger);
    }

    VkPhysicalDevice physicalDevice = pickPhysicalDevice(instance);

    return {instance, physicalDevice, debugUtilsMessenger, appInfo.apiVersion};
}
