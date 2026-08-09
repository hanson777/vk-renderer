#pragma once
#include "Render/vk_common.h"

namespace vk_instance {
    bool Init();
    void Shutdown();
    
    VkInstance GetInstance();    
    VkDebugUtilsMessengerEXT GetDebugMessenger();
    VkSurfaceKHR GetSurface();
}
