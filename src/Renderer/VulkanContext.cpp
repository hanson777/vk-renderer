#include "VulkanContext.h"
#include "Managers/VkInstanceManager.h"
#include <volk.h>
#include <iostream>

namespace VulkanContext {

	bool Init() {
		if (!volkInitialize()) {
            std::cerr << "[ERROR::VKCONTEXT] failed to initialize volk\n";
			return false;
		}
        
        if (!VkInstanceManager::Init()) { 
            std::cerr << "[ERROR::VK_CONTEXT] failed to create instance\n";
            return false;
        }

        return true;
	}
}
