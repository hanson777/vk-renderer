#pragma once
#include "Render/vk_common.h"
#include <vector>

namespace vk_swapchain {

    extern VkSwapchainKHR g_swapchain;
    extern bool g_recreateSwapchain;

    extern VkFormat g_swapchainImageFormat;
    extern VkExtent2D g_swapchainExtent;

    extern std::vector<VkImage> g_swapchainImages;
    extern std::vector<VkImageView> g_swapchainImageViews;

    extern VkImage g_depthImage;
    extern VkImageView g_depthImageView;
    extern VkFormat g_depthImageFormat;
    extern VmaAllocation g_depthImageAllocation;

    bool Init();
    void Shutdown();
    bool CreateSwapchain();
}
