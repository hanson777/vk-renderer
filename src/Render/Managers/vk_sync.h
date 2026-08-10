#pragma once
#include "Render/vk_common.h"
#include <cstdint>

namespace vk_sync {
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	extern std::vector<VkSemaphore> g_renderCompleteSemaphores;
	extern std::vector<VkSemaphore> g_imageAcquiredSemaphores;
	extern VkSemaphore g_timelineSemaphore;
	extern std::vector<VkCommandPool> g_commandPools;
	extern std::vector<VkCommandBuffer> g_commandBuffers;

	bool Init();
	void Shutdown();
}