#pragma once
#include "Render/vk_common.h"
#include <vector>

namespace vk_sync {
	extern std::vector<VkSemaphore> g_submit_semaphores;
	extern std::vector<VkSemaphore> g_acquire_semaphores;
	extern VkSemaphore g_timeline_semaphore;
    extern uint64_t g_timeline_value;
	extern std::vector<VkCommandPool> g_command_pools;
	extern std::vector<VkCommandBuffer> g_command_buffers;

	bool Init();
	void Shutdown();
}
