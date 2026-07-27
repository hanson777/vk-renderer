#pragma once
#include <vk_mem_alloc.h>

namespace VkMemoryManager {
	bool Init();
	void Shutdown();
	VmaAllocator GetAllocator();
}
