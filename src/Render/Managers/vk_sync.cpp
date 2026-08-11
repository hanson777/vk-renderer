#include "vk_sync.h"
#include "Render/vk_common.h"
#include "vk_swapchain.h"
#include "vk_device.h"
#include <vector>
#include <iostream>
#include <cstdint>

namespace vk_sync {

	std::vector<VkSemaphore> g_renderCompleteSemaphores;
	std::vector<VkSemaphore> g_imageAcquiredSemaphores;
	VkSemaphore g_timelineSemaphore = VK_NULL_HANDLE;
	std::vector<VkCommandPool> g_commandPools;
	std::vector<VkCommandBuffer> g_commandBuffers;

	bool Init() {
		const VkDevice& device = vk_device::GetDevice();

		VkSemaphoreTypeCreateInfo timelineSemaphoreTypeCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = MAX_FRAMES_IN_FLIGHT,
		};
		VkSemaphoreCreateInfo timelineSemaphoreCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &timelineSemaphoreTypeCreateInfo,
		};
		VkSemaphoreCreateInfo binarySemaphoreCreateInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

		if (vkCreateSemaphore(device, &timelineSemaphoreCreateInfo, nullptr, &g_timelineSemaphore) != VK_SUCCESS) {
			std::cerr << "[ERROR::SYNC_MANAGER] failed to create timeline semaphore\n";
			return false;
		};

		// semaphores for image acquisition
		g_imageAcquiredSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		for (VkSemaphore& semaphore : g_imageAcquiredSemaphores) {
			if (vkCreateSemaphore(device, &binarySemaphoreCreateInfo, nullptr, &semaphore) != VK_SUCCESS) {
				std::cerr << "[ERROR::SYNC_MANAGER] failed to create image acquired semaphore\n";
				return false;
			};
		}

		// semaphores for swapchain images
		g_renderCompleteSemaphores.resize(vk_swapchain::g_swapchainImages.size());
		for (VkSemaphore& semaphore : g_renderCompleteSemaphores) {
			if (vkCreateSemaphore(device, &binarySemaphoreCreateInfo, nullptr, &semaphore) != VK_SUCCESS) {
				std::cerr << "[ERROR::SYNC_MANAGER] failed to create render complete semaphore\n";
				return false;
			}
		}

		g_commandPools.resize(MAX_FRAMES_IN_FLIGHT);
		g_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandPoolCreateInfo cmdPoolCreateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = vk_device::GetQueueIndex(),
		};
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateCommandPool(device, &cmdPoolCreateInfo, nullptr, &g_commandPools[i]) != VK_SUCCESS) {
				std::cerr << "[ERROR::SYNC_MANAGER] failed to create command pool\n";
				return false;
			}

			VkCommandBufferAllocateInfo cmdAllocateInfo{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = g_commandPools[i],
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			if (vkAllocateCommandBuffers(device, &cmdAllocateInfo, &g_commandBuffers[i]) != VK_SUCCESS) {
				std::cerr << "[ERROR::SYNC_MANAGER] failed to create command buffer\n";
				return false;
			}
		}

		return true;
	}

	void Shutdown() {
		const VkDevice device = vk_device::GetDevice();
		if (g_timelineSemaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, g_timelineSemaphore, nullptr);
		}

		for (VkSemaphore& semaphore : g_renderCompleteSemaphores) {
			if (semaphore != VK_NULL_HANDLE) vkDestroySemaphore(device, semaphore, nullptr);
		}

		for (VkSemaphore& semaphore : g_imageAcquiredSemaphores) {
			if (semaphore != VK_NULL_HANDLE) vkDestroySemaphore(device, semaphore, nullptr);
		}

		for (VkCommandPool& cmdPool : g_commandPools) {
			if (cmdPool != VK_NULL_HANDLE) vkDestroyCommandPool(device, cmdPool, nullptr);
		}
	}
}