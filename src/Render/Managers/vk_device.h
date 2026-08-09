#pragma once
#include "Render/vk_common.h"

namespace vk_device {
	bool Init();
	VkPhysicalDevice GetPhysicalDevice();
	VkDevice GetDevice();
	VkQueue GetQueue();
}
