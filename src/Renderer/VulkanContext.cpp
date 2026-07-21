#include "VulkanContext.h"
#include <volk.h>
#include <iostream>
#include <cstdint>

namespace VulkanContext {

	constexpr uint32_t VK_VERSION = VK_API_VERSION_1_4;

	VkInstance g_vkInstance = nullptr;

	bool Init() {
		if (!volkInitialize()) {
			std::cout << "[ERROR::VKCONTEXT] failed to initialize volk" << std::endl;
			return false;
		}

		CreateInstance();
	}

	void CreateInstance() {

		VkApplicationInfo appInfo{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "hi",
			.apiVersion = VK_VERSION,
		};

		VkInstanceCreateInfo instanceInfo{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,

		};



		volkLoadInstance(g_vkInstance);
	}
}