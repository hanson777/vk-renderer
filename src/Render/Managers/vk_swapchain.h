#pragma once
#include "Render/vk_common.h"
#include <vector>

namespace vk_swapchain {

    extern VkSwapchainKHR g_swapchain;
    extern bool g_recreate_swapchain;

    extern VkFormat g_swapchain_image_format;
    extern VkExtent2D g_swapchain_extent;

    extern std::vector<VkImage> g_swapchain_images;
    extern std::vector<VkImageView> g_swapchain_image_views;

    extern VkImage g_depth_image;
    extern VkImageView g_depth_image_view;
    extern VkFormat g_depth_image_format;
    extern VmaAllocation g_depth_image_allocation;

    bool Init();
    void Shutdown();
    bool CreateSwapchain();
    void RecreateSwapchain();
}
