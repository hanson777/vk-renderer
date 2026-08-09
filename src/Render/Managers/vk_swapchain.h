#pragma once
#include "Render/vk_common.h"
#include <vector>

namespace vk_swapchain {
    bool Init();
    void Shutdown();
    
    bool RecreateSwapchain();
    
    VkFormat* GetSwapchainImageFormat();
    VkFormat GetDepthImageFormat();
    std::vector<VkImage>& GetSwapchainImages();
}
