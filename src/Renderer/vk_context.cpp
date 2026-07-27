#include "vk_context.h"
#include "Managers/vk_instance.h"
#include "Managers/vk_device.h"
#include "Managers/vk_memory.h"
#include "Managers/vk_swapchain.h"
#include <volk.h>
#include <iostream>

namespace VkContext {

	bool Init() {
        VkResult result = volkInitialize();
		if (result != VK_SUCCESS) {
            std::cerr << "[ERROR::VKCONTEXT] failed to initialize volk: " << result << '\n';
			return false;
		}
        
        if (!VkInstanceManager::Init()) { 
            std::cerr << "[ERROR::VK_CONTEXT] VkInstanceManager::Init() failed\n";
            return false;
        }

        if (!VkDeviceManager::Init()) {
            std::cerr << "[ERROR::VK_CONTEXT] VkDeviceManager::Init() failed\n";
            return false;
        }

        if (!VkMemoryManager::Init()) {
            std::cerr << "[ERROR::VK_CONTEXT] VkMemoryManager::Init() failed\n";
            return false;
        }

        if (!VkSwapchainManager::Init()) {
            std::cerr << "[ERROR::VK_CONTEXT] VkSwapchainManager::Init() failed\n";
            return false;
        }

        return true;
	}

    void Shutdown() {
        VkMemoryManager::Shutdown();
        VkInstanceManager::Shutdown();
    }
}
