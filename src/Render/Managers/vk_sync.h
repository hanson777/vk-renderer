#pragma once
#include "Render/vk_common.h"
#include <cstdint>
#include <vector>

namespace vk_sync {
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	extern std::vector<VkSemaphore> g_submit_semaphores;
	extern std::vector<VkSemaphore> g_acquire_semaphores;
	extern VkSemaphore g_timeline_semaphore;
	extern std::vector<VkCommandPool> g_command_pools;
	extern std::vector<VkCommandBuffer> g_command_buffers;

	bool Init();
	void Shutdown();
}