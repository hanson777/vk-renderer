#pragma once
#include "Render/vk_common.h"

namespace VkSwapchainManager {
    bool Init();
    void Shutdown();
    
    bool RecreateSwapchain();
    
    VkFormat* GetSwapchainImageFormat();
    VkFormat GetDepthImageFormat();
}
