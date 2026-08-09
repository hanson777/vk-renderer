#include "vk_sync.h"
#include "Render/vk_common.h"
#include "vk_swapchain.h"
#include "vk_device.h"
#include <vector>
#include <iostream>
#include <cstdint>

namespace vk_sync {

	constexpr uint32_t maxFramesInFlight = 2;

	std::vector<VkSemaphore> g_renderCompleteSemaphores;
	std::vector<VkSemaphore> g_imageAcquiredSemaphores(maxFramesInFlight);
	VkSemaphore g_timelineSemaphore = VK_NULL_HANDLE;

	bool Init() {
		const VkDevice device = vk_device::GetDevice();

		VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = maxFramesInFlight,
		};
		VkSemaphoreCreateInfo semaphoreCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &semaphoreTypeCreateInfo,
		};

		if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &g_timelineSemaphore) != VK_SUCCESS) {
			std::cerr << "[ERROR::VK_SYNC_MANAGER] failed to create timeline semaphore\n";
			return false;
		};

		for (VkSemaphore& semaphore : g_imageAcquiredSemaphores) {
			VkSemaphoreCreateInfo semaphoreCreateInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore) != VK_SUCCESS) {
				std::cerr << "[ERROR::VK_SYNC_MANAGER] failed to create image acquired semaphore\n";
				return false;
			};
		}

		g_renderCompleteSemaphores.resize(vk_swapchain::GetSwapchainImages().size());
		for (VkSemaphore& semaphore : g_renderCompleteSemaphores) {
			if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore) != VK_SUCCESS) {
				std::cerr << "[ERROR::SWAPCHAIN_MANAGER] failed to create render complete semaphore\n";
				return false;
			}
		}

		return true;
	}
}

