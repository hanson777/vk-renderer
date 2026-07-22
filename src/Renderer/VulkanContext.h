#pragma once
#include <volk.h>

namespace VulkanContext {
	
	extern VkInstance g_instance;

	bool Init();
	void CreateInstance();
}
