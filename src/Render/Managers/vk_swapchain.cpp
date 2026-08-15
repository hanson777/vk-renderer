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
    bool g_recreate_swapchain = false;

    VkFormat g_swapchain_image_format = VK_FORMAT_UNDEFINED;
    VkExtent2D g_swapchain_extent{};

    std::vector<VkImage> g_swapchain_images;
    std::vector<VkImageView> g_swapchain_image_views;

    VkImage g_depth_image = VK_NULL_HANDLE;
    VkImageView g_depth_image_view = VK_NULL_HANDLE;
    VkFormat g_depth_image_format = VK_FORMAT_UNDEFINED;
    VmaAllocation g_depth_image_allocation = VK_NULL_HANDLE;

    static bool getSwapchainImages();
    static bool getDepthImages();

    static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    static bool supportsImageFormat(const VkFormat format);

    bool Init() {
        if (!CreateSwapchain()) return false;
        return true;
    }

    void Shutdown() {
        const VkDevice& device = vk_device::GetDevice();
        for (VkImageView& swapchain_image_view : g_swapchain_image_views) {
            if (swapchain_image_view != VK_NULL_HANDLE) {
                vkDestroyImageView(device, swapchain_image_view, nullptr);
            }
        }
        g_swapchain_image_views.clear();

        if (g_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, g_swapchain, nullptr);
            g_swapchain = VK_NULL_HANDLE;
            g_swapchain_images.clear();
        }

        if (g_depth_image_view != VK_NULL_HANDLE) {
            vkDestroyImageView(device, g_depth_image_view, nullptr);
            g_depth_image_view = VK_NULL_HANDLE;
        }

        if (g_depth_image != VK_NULL_HANDLE) {
            vmaDestroyImage(vk_memory::GetAllocator(), g_depth_image, g_depth_image_allocation);
            g_depth_image = VK_NULL_HANDLE;
        }
    }

    bool CreateSwapchain() {
        const VkPhysicalDevice physical_device = vk_device::GetPhysicalDevice();
        const VkDevice device = vk_device::GetDevice();
        const VkSurfaceKHR surface = vk_instance::GetSurface();

        VkSurfaceCapabilitiesKHR surface_caps{};
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_caps) != VK_SUCCESS) {
            std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to get surface capabilities\n";
            return false;
        }

        const VkFormat image_format = VK_FORMAT_B8G8R8A8_SRGB;
        if (!supportsImageFormat(image_format)) {
            std::cerr << "[ERROR::SWAPCHAIN_MANAGER] surface does not support requested image format\n";
            return false;
        }
        g_swapchain_image_format = image_format;

        g_swapchain_extent = chooseSwapExtent(surface_caps);
        VkSwapchainCreateInfoKHR swapchain_create_info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = surface_caps.minImageCount + 1,
            .imageFormat = image_format,
            .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
            .imageExtent{.width = g_swapchain_extent.width, .height = g_swapchain_extent.height },
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = surface_caps.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
            .clipped = VK_TRUE,
        };

        if (vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &g_swapchain) != VK_SUCCESS) {
            std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to create swapchain\n";
            return false;
        }

        if (!getSwapchainImages()) return false;
        if (!getDepthImages()) return false;

        return true;
    }

    void RecreateSwapchain() {
        Shutdown();
        CreateSwapchain();
    }

    static bool getSwapchainImages() {
        VkDevice device = vk_device::GetDevice();
        VkPhysicalDevice physical_device = vk_device::GetPhysicalDevice();

        uint32_t image_count = 0;
        vkGetSwapchainImagesKHR(device, g_swapchain, &image_count, nullptr);
        g_swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(device, g_swapchain, &image_count, g_swapchain_images.data());
        g_swapchain_image_views.resize(image_count);

        for (size_t i = 0; i < g_swapchain_images.size(); i++) {
            VkImageViewCreateInfo image_view_create_info{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = g_swapchain_images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = g_swapchain_image_format,
                .subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };

            if (vkCreateImageView(device, &image_view_create_info, nullptr, &g_swapchain_image_views[i]) != VK_SUCCESS) {
                std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to create swapchain image view\n";
                return false;
            }
        }

        return true;
    }

    static bool getDepthImages() {
        VkDevice device = vk_device::GetDevice();
        VkPhysicalDevice physical_device = vk_device::GetPhysicalDevice();

        std::vector<VkFormat> depth_formats{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
        for (const VkFormat& format : depth_formats) {
            VkFormatProperties2 format_properties{ .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
            vkGetPhysicalDeviceFormatProperties2(physical_device, format, &format_properties);
            if (format_properties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                g_depth_image_format = format;
                break;
            }
        }

        VkImageCreateInfo depth_image_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = g_depth_image_format,
            .extent{.width = g_swapchain_extent.width, .height = g_swapchain_extent.height, .depth = 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VmaAllocationCreateInfo alloc_create_info{
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        if (vmaCreateImage(vk_memory::GetAllocator(), &depth_image_create_info, &alloc_create_info, &g_depth_image, &g_depth_image_allocation, nullptr) != VK_SUCCESS) {
            std::cerr << "[ERROR::SWAPCHAIN] error creating and allocating depth image\n";
        }

        VkImageViewCreateInfo depth_image_view_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = g_depth_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = g_depth_image_format,
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, 
                              .levelCount = 1, 
                              .layerCount = 1,
            },
        };

        if (vkCreateImageView(device, &depth_image_view_info, nullptr, &g_depth_image_view) != VK_SUCCESS) {
            std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to create depth image view\n";
            return false;
        }

        return true;
    }

    static bool supportsImageFormat(const VkFormat format) {
        VkPhysicalDevice physical_device = vk_device::GetPhysicalDevice();
        VkSurfaceKHR surface = vk_instance::GetSurface();

        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, surface_formats.data());

        for (const auto& surface_format : surface_formats) {
            if (surface_format.format == format) {
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

    VkFormat* GetSwapchainImageFormat() { return &g_swapchain_image_format; }

    VkImage& GetDepthImage() { return g_depth_image; }
    
    VkFormat GetDepthImageFormat() { return g_depth_image_format; }
}
