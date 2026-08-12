#include "vk_context.h"
#include "vk_common.h"
#include "Managers/vk_instance.h"
#include "Managers/vk_device.h"
#include "Managers/vk_memory.h"
#include "Managers/vk_swapchain.h"
#include "Managers/vk_pipeline.h"
#include "Managers/vk_sync.h"
#include <iostream>

namespace vk_context {

	bool Init() {
        VkResult result = volkInitialize();
		if (result != VK_SUCCESS) {
            std::cerr << "[ERROR::VKCONTEXT] failed to initialize volk: " << result << '\n';
			return false;
		}
        
        if (!vk_instance::Init()) { 
            std::cerr << "[ERROR::VK_CONTEXT] vk_instance::Init() failed\n";
            return false;
        }

        if (!vk_device::Init()) {
            std::cerr << "[ERROR::VK_CONTEXT] vk_device::Init() failed\n";
            return false;
        }

        if (!vk_memory::Init()) {
            std::cerr << "[ERROR::VK_CONTEXT] vk_memory::Init() failed\n";
            return false;
        }

        if (!vk_swapchain::Init()) {
            std::cerr << "[ERROR::VK_CONTEXT] vk_swapchain::Init() failed\n";
            return false;
        }

        if (!vk_pipeline::Init()) {
            std::cerr << "[ERROR::VK_CONTEXT] vk_pipeline::Init() failed\n";
            return false;
        }

        if (!vk_sync::Init()) {
            std::cerr << "[ERROR::VK_CONTEXT] vk_sync::Init() failed\n";
            return false;
        }

        return true;
	}

    void Shutdown() {
        vkDeviceWaitIdle(vk_device::GetDevice());
        vk_sync::Shutdown();
        vk_pipeline::Shutdown();
        vk_swapchain::Shutdown();
        vk_memory::Shutdown();
        vk_device::Shutdown();
        vk_instance::Shutdown();
    }
}
