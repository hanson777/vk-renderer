#pragma once
#include "Render/vk_common.h"

namespace VkDeviceManager {
	bool Init();
	VkPhysicalDevice GetPhysicalDevice();
	VkDevice GetLogicalDevice();
	VkQueue GetQueue();
}
