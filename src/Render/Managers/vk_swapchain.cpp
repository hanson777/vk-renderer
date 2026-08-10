#include "vk_swapchain.h"
#include "Render/vk_common.h"
#include "vk_instance.h"
#include "vk_device.h"
#include "vk_memory.h"
#include "Core/Window.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdint>

namespace vk_swapchain {

    VkSwapchainKHR g_swapchain = VK_NULL_HANDLE;
    bool g_recreateSwapchain = false;

    VkFormat g_swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D g_swapchainExtent{};

    std::vector<VkImage> g_swapchainImages;
    std::vector<VkImageView> g_swapchainImageViews;

    VkImage g_depthImage = VK_NULL_HANDLE;
    VkImageView g_depthImageView = VK_NULL_HANDLE;
    VkFormat g_depthImageFormat = VK_FORMAT_UNDEFINED;
    VmaAllocation g_depthImageAllocation = VK_NULL_HANDLE;

    static bool getSwapchainImages();
    static bool getDepthImages();

    static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    static bool supportsImageFormat(const VkFormat format);

    bool Init() {
        if (!CreateSwapchain()) return false;
        if (!getSwapchainImages()) return false;
        if (!getDepthImages()) return false;
        return true;
    }

    bool CreateSwapchain() {
        const VkPhysicalDevice physicalDevice = vk_device::GetPhysicalDevice();
        const VkDevice device = vk_device::GetDevice();
        const VkSurfaceKHR surface = vk_instance::GetSurface();

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

        g_swapchainExtent = chooseSwapExtent(surfaceCaps);
        VkSwapchainCreateInfoKHR swapchainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = surfaceCaps.minImageCount,
            .imageFormat = imageFormat,
            .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
            .imageExtent{.width = g_swapchainExtent.width, .height = g_swapchainExtent.height },
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = surfaceCaps.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
            .clipped = VK_TRUE,
        };

        if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &g_swapchain) != VK_SUCCESS) {
            std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to create swapchain\n";
            return false;
        }

        return true;
    }

    void Shutdown() {
        const VkDevice& device = vk_device::GetDevice();
        for (VkImageView& swapchainImageView : g_swapchainImageViews) {
            if (swapchainImageView != VK_NULL_HANDLE) {
                vkDestroyImageView(device, swapchainImageView, nullptr);
            }
        }
        g_swapchainImageViews.clear();

        if (g_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, g_swapchain, nullptr);
            g_swapchain = VK_NULL_HANDLE;
            g_swapchainImages.clear();
        }

        if (g_depthImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, g_depthImageView, nullptr);
            g_depthImageView = VK_NULL_HANDLE;
        }

        if (g_depthImage != VK_NULL_HANDLE) {
            vmaDestroyImage(vk_memory::GetAllocator(), g_depthImage, g_depthImageAllocation);
            g_depthImage = VK_NULL_HANDLE;
        }
    }

    static bool getSwapchainImages() {
        VkDevice device = vk_device::GetDevice();
        VkPhysicalDevice physicalDevice = vk_device::GetPhysicalDevice();

        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(device, g_swapchain, &imageCount, nullptr);
        g_swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, g_swapchain, &imageCount, g_swapchainImages.data());
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

            if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &g_swapchainImageViews[i]) != VK_SUCCESS) {
                std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to create swapchain image view\n";
                return false;
            }
        }

        return true;
    }

    static bool getDepthImages() {
        VkDevice device = vk_device::GetDevice();
        VkPhysicalDevice physicalDevice = vk_device::GetPhysicalDevice();

        std::vector<VkFormat> depthFormats{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
        for (const VkFormat& format : depthFormats) {
            VkFormatProperties2 formatProperties{ .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
            vkGetPhysicalDeviceFormatProperties2(physicalDevice, format, &formatProperties);
            if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                g_depthImageFormat = format;
                break;
            }
        }

        VkImageCreateInfo depthImageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = g_depthImageFormat,
            .extent{.width = g_swapchainExtent.width, .height = g_swapchainExtent.height, .depth = 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VmaAllocationCreateInfo allocCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        if (vmaCreateImage(vk_memory::GetAllocator(), &depthImageCreateInfo, &allocCreateInfo, &g_depthImage, &g_depthImageAllocation, nullptr) != VK_SUCCESS) {
            std::cerr << "[ERROR::SWAPCHAIN] error creating and allocating depth image\n";
        }

        VkImageViewCreateInfo depthImageViewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = g_depthImage,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = g_depthImageFormat,
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, 
                              .levelCount = 1, 
                              .layerCount = 1,
            },
        };

        if (vkCreateImageView(device, &depthImageViewInfo, nullptr, &g_depthImageView) != VK_SUCCESS) {
            std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to create depth image view\n";
            return false;
        }

        return true;
    }

    static bool supportsImageFormat(const VkFormat format) {
        VkPhysicalDevice physicalDevice = vk_device::GetPhysicalDevice();
        VkSurfaceKHR surface = vk_instance::GetSurface();

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

    static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        int width, height;
        glfwGetFramebufferSize(Window::GetWindowPointer(), &width, &height);

        return {
            std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }

    VkSwapchainKHR GetSwapchain() { return g_swapchain; }

    VkFormat* GetSwapchainImageFormat() { return &g_swapchainImageFormat; }

    VkImage& GetDepthImage() { return g_depthImage; }
    
    VkFormat GetDepthImageFormat() { return g_depthImageFormat; }

    std::vector<VkImage>& GetSwapchainImages() { return g_swapchainImages; }
}