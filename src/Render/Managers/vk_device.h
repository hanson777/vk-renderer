#pragma once
#include "Render/vk_common.h"
#include <cstdint>

namespace vk_device {
	bool Init();
	void Shutdown();
	const VkPhysicalDevice& GetPhysicalDevice();
	const VkDevice& GetDevice();
	VkQueue GetQueue();
	uint32_t GetQueueIndex();
}
