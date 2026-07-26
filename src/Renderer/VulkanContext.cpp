#include "VulkanContext.h"
#include "Managers/VkInstanceManager.h"
#include "Managers/VkDeviceManager.h"
#include "Managers/VkMemoryManager.h"
#include "Managers/VkSwapchainManager.h"
#include <volk.h>
#include <iostream>

namespace VulkanContext {

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
