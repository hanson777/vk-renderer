#pragma once
#include "Render/vk_common.h"
#include <vector>
#include <cstdint>

namespace vk_swapchain {

    VkSwapchainKHR g_swapchain;
    extern bool g_recreateSwapchain;

    VkFormat g_swapchainImageFormat;
    VkExtent2D g_swapchainExtent;

    std::vector<VkImage> g_swapchainImages;
    std::vector<VkImageView> g_swapchainImageViews;

    VkImage g_depthImage;
    VkImageView g_depthImageView;
    VkFormat g_depthImageFormat;
    VmaAllocation g_depthImageAllocation;

    bool Init();
    void Shutdown();
    bool CreateSwapchain();
    
    VkSwapchainKHR GetSwapchain();
    VkFormat* GetSwapchainImageFormat();
    VkImage& GetDepthImage();
    VkFormat GetDepthImageFormat();
    std::vector<VkImage>& GetSwapchainImages();
}
