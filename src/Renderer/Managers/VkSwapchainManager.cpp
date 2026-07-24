#include "VkSwapchainManager.h"
#include "VkInstanceManager.h"
#include "VkDeviceManager.h"
#include "vulkan/vulkan_core.h"
#include <volk.h>
#include <iostream>

namespace VkSwapchainManager {

    VkSwapchainKHR g_swapchain = VK_NULL_HANDLE;
    VkFormat g_swapchainImageFormat = VK_FORMAT_UNDEFINED;
    std::vector<VkImage> g_swapchainImages;
    std::vector<VkImageView> g_swapchainImageViews;
    
    static bool supportsImageFormat(const VkFormat format) { 
        VkPhysicalDevice physicalDevice = VkDeviceManager::GetPhysicalDevice();
        VkSurfaceKHR surface = VkInstanceManager::GetSurface();

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());
        
        for (const auto& surfaceFormat : surfaceFormats) {
            if (surfaceFormat.format == format) {
                return true;
            }
        }

        return false;
    }
    
    static bool CreateSwapchain() {
        VkPhysicalDevice physicalDevice = VkDeviceManager::GetPhysicalDevice();
        VkDevice logicalDevice = VkDeviceManager::GetLogicalDevice();
        VkSurfaceKHR surface = VkInstanceManager::GetSurface();

        VkSurfaceCapabilitiesKHR surfaceCaps{};
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps) != VK_SUCCESS) {
            std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to get surface capabilities\n";
            return false;
        }

        const VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
        if (!supportsImageFormat(imageFormat)) {
            std::cerr << "[ERROR::SWAPCHAIN_MANAGER] surface does not support requested image format\n";
            return false;
        }
        g_swapchainImageFormat = imageFormat;
        
        VkExtent2D swapchainExtent = surfaceCaps.currentExtent;
        VkSwapchainCreateInfoKHR swapchainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface, 
            .minImageCount = surfaceCaps.minImageCount,
            .imageFormat = imageFormat,
            .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
            .imageExtent{.width = swapchainExtent.width, .height = swapchainExtent.height },
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = surfaceCaps.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        };

        if (vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &g_swapchain) != VK_SUCCESS) {
            std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to create swapchain\n";
            return false;
        }

        return true;
    }
    
    static bool getSwapchainImages() {
        VkDevice logicalDevice = VkDeviceManager::GetLogicalDevice();

        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(logicalDevice, g_swapchain, &imageCount, nullptr);
        g_swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(logicalDevice, g_swapchain, &imageCount, nullptr);
        g_swapchainImageViews.resize(imageCount);
        
        for (size_t i = 0; i < g_swapchainImages.size(); i++) {
            VkImageViewCreateInfo imageViewCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = g_swapchainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = g_swapchainImageFormat,
                .subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };
            
            if (vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &g_swapchainImageViews[i]) != VK_SUCCESS) {
                std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to create swapchain image view\n";
                return false;
            }
        }

        return true;
    }

    bool Init() {
        if (!CreateSwapchain()) return false;
        return true;
    }
}
