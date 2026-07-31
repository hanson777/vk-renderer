#pragma once
#include "Render/vk_common.h"

namespace VkMemoryManager {
	bool Init();
	void Shutdown();
	VmaAllocator GetAllocator();
}
