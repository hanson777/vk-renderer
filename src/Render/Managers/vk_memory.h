#pragma once
#include "Render/vk_common.h"

namespace vk_memory {
	bool Init();
	void Shutdown();
	const VmaAllocator& GetAllocator();
}
