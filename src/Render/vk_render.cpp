#include "vk_render.h"
#include "vk_common.h"
#include "Managers/vk_device.h"
#include "Managers/vk_swapchain.h"
#include "Managers/vk_pipeline.h"
#include "Managers/vk_sync.h"
#include <cstdint>
#include <vector>
#include <iostream>

namespace vk_render {

	uint32_t g_frameIndex = 0;
	uint64_t g_nextSignalValue = vk_sync::MAX_FRAMES_IN_FLIGHT + 1;

	void Render() {
		const VkDevice& device = vk_device::GetDevice();

		if (vk_swapchain::g_recreateSwapchain) {
			vkDeviceWaitIdle(device);
			vk_swapchain::Shutdown();
			vk_swapchain::CreateSwapchain();
			vk_swapchain::g_recreateSwapchain = false;
		}

		const uint32_t currentFrameIdx = g_frameIndex % vk_sync::MAX_FRAMES_IN_FLIGHT;
		const uint64_t signalValue = g_nextSignalValue;
		const uint64_t waitValue = signalValue - vk_sync::MAX_FRAMES_IN_FLIGHT;
		g_frameIndex++;
		g_nextSignalValue++;

		// wait for semaphore
		VkSemaphoreWaitInfo semaphoreWaitInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &vk_sync::g_timelineSemaphore,
			.pValues = &waitValue,
		};
		vkWaitSemaphores(device, &semaphoreWaitInfo, UINT64_MAX);

		VkCommandPool cmdPool = vk_sync::g_commandPools[currentFrameIdx];
		vkResetCommandPool(device, cmdPool, 0);

		// request next image
		VkSemaphore imageAcquiredSemaphore = vk_sync::g_imageAcquiredSemaphores[currentFrameIdx];
		uint32_t imageIndex = 0;
		VkResult result = vkAcquireNextImageKHR(device, vk_swapchain::g_swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			vk_swapchain::g_recreateSwapchain = true;
			return;
		}
		else if (result == VK_SUBOPTIMAL_KHR) {
			vk_swapchain::g_recreateSwapchain = true;
		}

		// begin recording commands
		VkCommandBufferBeginInfo cmdBufferBeginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		VkCommandBuffer& cmdBuffer = vk_sync::g_commandBuffers[currentFrameIdx];
		vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);

		// transition image layout
		std::vector<VkImageMemoryBarrier2> layoutBarriers{
			// swapchain image
			VkImageMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
				.srcAccessMask = VK_ACCESS_2_NONE,
				.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.image = vk_swapchain::g_swapchainImages[imageIndex],
				.subresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			},
			// depth image
			VkImageMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
				.srcAccessMask = VK_ACCESS_2_NONE,
				.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
				.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				.image = vk_swapchain::g_depthImage,
				.subresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			},
		};
		VkDependencyInfo depInfo{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = static_cast<uint32_t>(layoutBarriers.size()),
			.pImageMemoryBarriers = layoutBarriers.data(),
		};
		vkCmdPipelineBarrier2(vk_sync::g_commandBuffers[currentFrameIdx], &depInfo);

		VkRenderingAttachmentInfo colorAttachmentInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = vk_swapchain::g_swapchainImageViews[imageIndex],
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue{.color{0.f, 0.f, 0.5f, 1}},
		};
		VkRenderingAttachmentInfo depthAttachmentInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = vk_swapchain::g_depthImageView,
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.clearValue{.depthStencil{1.f, 0}},
		};

		VkRenderingInfo renderingInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea{
				.offset{.x = 0, .y = 0 },
				.extent = vk_swapchain::g_swapchainExtent,
			},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo,
			.pDepthAttachment = &depthAttachmentInfo,
		};

		vkCmdBeginRendering(cmdBuffer, &renderingInfo);
		{
			VkViewport viewport{
				.x = 0,
				.y = 0,
				.width = static_cast<float>(vk_swapchain::g_swapchainExtent.width),
				.height = static_cast<float>(vk_swapchain::g_swapchainExtent.height),
			};
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

			VkRect2D scissor{
				.offset{.x = 0, .y = 0},
				.extent = vk_swapchain::g_swapchainExtent,
			};
			vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

			vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline::g_pipeline);
			vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
			std::cout << "Calling vkCmdDraw()...\n";
		}
		vkCmdEndRendering(cmdBuffer);

		VkImageMemoryBarrier2 presentLayoutBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
			.dstAccessMask = VK_ACCESS_2_NONE,
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.image = vk_swapchain::g_swapchainImages[imageIndex],
			.subresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		VkDependencyInfo presentDepInfo{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &presentLayoutBarrier,
		};
		vkCmdPipelineBarrier2(cmdBuffer, &presentDepInfo);

		vkEndCommandBuffer(cmdBuffer);

		VkSemaphoreSubmitInfo imageAcquireWaitInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = imageAcquiredSemaphore,
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		};
		std::vector<VkSemaphoreSubmitInfo> semaphoreSignals{
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = vk_sync::g_renderCompleteSemaphores[imageIndex],
				.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
			},
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = vk_sync::g_timelineSemaphore,
				.value = signalValue,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			},
		};

		VkCommandBufferSubmitInfo cmdBufferSubmitInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = cmdBuffer,
		};
		VkSubmitInfo2 submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = 1,
			.pWaitSemaphoreInfos = &imageAcquireWaitInfo,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &cmdBufferSubmitInfo,
			.signalSemaphoreInfoCount = static_cast<uint32_t>(semaphoreSignals.size()),
			.pSignalSemaphoreInfos = semaphoreSignals.data(),
		};
		const VkQueue& queue = vk_device::GetQueue();
		vkQueueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE);

		VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &vk_sync::g_renderCompleteSemaphores[imageIndex],
			.swapchainCount = 1,
			.pSwapchains = &vk_swapchain::g_swapchain,
			.pImageIndices = &imageIndex,
			.pResults = nullptr,
		};

		vkQueuePresentKHR(queue, &presentInfo);
	}

}
