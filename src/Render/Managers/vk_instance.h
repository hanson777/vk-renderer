#pragma once
#include "Render/vk_common.h"

namespace VkInstanceManager {
    bool Init();
    void Shutdown();
    
    VkInstance GetInstance();    
    VkDebugUtilsMessengerEXT GetDebugMessenger();
    VkSurfaceKHR GetSurface();
}
