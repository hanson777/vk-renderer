#include <volk.h>

namespace VkInstanceManager {
    bool Init();
    void Shutdown();
    
    VkInstance GetInstance();    
    VkDebugUtilsMessengerEXT GetDebugMessenger();
    VkSurfaceKHR GetSurface();
}
