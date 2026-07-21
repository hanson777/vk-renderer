#include "VulkanContext.h"
#include "../Core/Window.h"
#include <volk.h>
#include <iostream>
#include <cstdint>
#include <vector>

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

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = Window::GetInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> requestedExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
		for (int i = 0; i < glfwExtensionCount; i++) {
			requestedExtensions.push_back(glfwExtensions[i]);
		}

		VkInstanceCreateInfo instanceInfo{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo,
			.enabledExtensionCount = glfwExtensionCount,
			.ppEnabledExtensionNames = glfwExtensions,
		};



		volkLoadInstance(g_vkInstance);
	}
}