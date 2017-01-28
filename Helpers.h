#pragma once
#ifndef HELPERS_H
#define HELPERS_H

#include <vector>
#include <sstream>
#include <stdexcept>
#include <vulkan/vulkan.h>





inline VkResult assert_success(VkResult res)
{
    if (res != VK_SUCCESS) {
        std::stringstream ss;
        ss << "VkResult " << res << " returned";
        throw std::runtime_error(ss.str());
    }

    return res;
}

inline VkResult enumerate(const char *layer, std::vector<VkExtensionProperties> &exts)
{
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);

    exts.resize(count);
    return vkEnumerateInstanceExtensionProperties(layer, &count, exts.data());
}

inline VkResult enumerate(VkPhysicalDevice phy, const char *layer, std::vector<VkExtensionProperties> &exts)
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(phy, layer, &count, nullptr);

    exts.resize(count);
    return vkEnumerateDeviceExtensionProperties(phy, layer, &count, exts.data());
}

inline VkResult enumerate(VkInstance instance, std::vector<VkPhysicalDevice> &phys)
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);

    phys.resize(count);
    return vkEnumeratePhysicalDevices(instance, &count, phys.data());
}

inline VkResult enumerate(std::vector<VkLayerProperties> &layer_props)
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    layer_props.resize(count);
    return vkEnumerateInstanceLayerProperties(&count, layer_props.data());
}

inline VkResult get(VkPhysicalDevice phy, std::vector<VkQueueFamilyProperties> &queues)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phy, &count, nullptr);

    queues.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(phy, &count, queues.data());

    return VK_SUCCESS;
}

inline VkResult get(VkPhysicalDevice phy, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR> &formats)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(phy, surface, &count, nullptr);

    formats.resize(count);
    return vkGetPhysicalDeviceSurfaceFormatsKHR(phy, surface, &count, formats.data());
}

inline VkResult get(VkPhysicalDevice phy, VkSurfaceKHR surface, std::vector<VkPresentModeKHR> &modes)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(phy, surface, &count, nullptr);

    modes.resize(count);
    return vkGetPhysicalDeviceSurfacePresentModesKHR(phy, surface, &count, modes.data());
}

inline VkResult get(VkDevice dev, VkSwapchainKHR swapchain, std::vector<VkImage> &images)
{
    uint32_t count = 0;
    vkGetSwapchainImagesKHR(dev, swapchain, &count, nullptr);

    images.resize(count);
    return vkGetSwapchainImagesKHR(dev, swapchain, &count, images.data());
}



#endif // HELPERS_H
