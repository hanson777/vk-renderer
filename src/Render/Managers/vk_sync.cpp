#include "vk_sync.h"
#include "Render/vk_common.h"
#include "vk_swapchain.h"
#include "vk_device.h"
#include <vector>
#include <cstdint>

namespace vk_sync {

	std::vector<VkSemaphore> g_submit_semaphores;
	std::vector<VkSemaphore> g_acquire_semaphores;
	VkSemaphore g_timeline_semaphore = VK_NULL_HANDLE;
    uint64_t g_timeline_value = MAX_FRAMES_IN_FLIGHT;
	std::vector<VkCommandPool> g_command_pools;
	std::vector<VkCommandBuffer> g_command_buffers;

	bool Init() {
		const VkDevice& device = vk_device::GetDevice();

		VkSemaphoreTypeCreateInfo timeline_semaphore_type_create_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = g_timeline_value,
		};
		VkSemaphoreCreateInfo timeline_semaphore_create_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &timeline_semaphore_type_create_info,
		};
		VkSemaphoreCreateInfo binary_semaphore_create_info{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

		VK_CHECK(vkCreateSemaphore(device, &timeline_semaphore_create_info, nullptr, &g_timeline_semaphore));

		// semaphores for image acquisition
		g_acquire_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		for (VkSemaphore& semaphore : g_acquire_semaphores) {
            VK_CHECK(vkCreateSemaphore(device, &binary_semaphore_create_info, nullptr, &semaphore));
		}

		// semaphores for swapchain images
		g_submit_semaphores.resize(vk_swapchain::g_swapchain_images.size());
		for (VkSemaphore& semaphore : g_submit_semaphores) {
            VK_CHECK(vkCreateSemaphore(device, &binary_semaphore_create_info, nullptr, &semaphore));
		}

		g_command_pools.resize(MAX_FRAMES_IN_FLIGHT);
		g_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandPoolCreateInfo cmd_pool_create_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = vk_device::GetQueueIndex(),
		};
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK(vkCreateCommandPool(device, &cmd_pool_create_info, nullptr, &g_command_pools[i]));

			VkCommandBufferAllocateInfo cmd_allocate_info{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = g_command_pools[i],
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			VK_CHECK(vkAllocateCommandBuffers(device, &cmd_allocate_info, &g_command_buffers[i]));
		}

		return true;
	}

	void Shutdown() {
		const VkDevice& device = vk_device::GetDevice();
		if (g_timeline_semaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, g_timeline_semaphore, nullptr);
		}

		for (VkSemaphore& semaphore : g_submit_semaphores) {
			if (semaphore != VK_NULL_HANDLE) vkDestroySemaphore(device, semaphore, nullptr);
		}

		for (VkSemaphore& semaphore : g_acquire_semaphores) {
			if (semaphore != VK_NULL_HANDLE) vkDestroySemaphore(device, semaphore, nullptr);
		}

		for (int i = 0; i < g_command_buffers.size(); i++) {
			if (g_command_buffers[i] != VK_NULL_HANDLE) vkFreeCommandBuffers(device, g_command_pools[i], 1, &g_command_buffers[i]);
		}

		for (VkCommandPool& pool : g_command_pools) {
			if (pool != VK_NULL_HANDLE) vkDestroyCommandPool(device, pool, nullptr);
		}

	}
}
