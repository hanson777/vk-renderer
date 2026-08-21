#include "Renderer.h"
#include "vk_common.h"
#include "Managers/vk_device.h"
#include "Managers/vk_swapchain.h"
#include "Managers/vk_pipeline.h"
#include "Managers/vk_sync.h"
#include "Render/Types/Shader.h"
#include "Camera/Arcball.h"
#include "Camera/Orbit.h"
#include "Core/Window.h"
#include <cstdint>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Renderer {

	uint32_t g_frame_count = 0;
	uint64_t g_next_signal_value = vk_sync::MAX_FRAMES_IN_FLIGHT + 1;

	void Render() {
		const VkDevice& device = vk_device::GetDevice();

		if (vk_swapchain::g_recreate_swapchain) {
			vkDeviceWaitIdle(device);
			vk_swapchain::RecreateSwapchain();
			vk_swapchain::g_recreate_swapchain = false;
		}

		const uint32_t frame_index = g_frame_count % vk_sync::MAX_FRAMES_IN_FLIGHT;
		const uint64_t signal_value = g_next_signal_value;
		const uint64_t wait_value = signal_value - vk_sync::MAX_FRAMES_IN_FLIGHT;

		// wait for semaphore (replaces wait for fence)
		VkSemaphoreWaitInfo wait_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &vk_sync::g_timeline_semaphore,
			.pValues = &wait_value,
		};
		vkWaitSemaphores(device, &wait_info, UINT64_MAX);

		VkCommandPool command_pool = vk_sync::g_command_pools[frame_index];
		vkResetCommandPool(device, command_pool, 0);

		// request next image
		uint32_t image_index = 0;
		VkSemaphore acquire_semaphore = vk_sync::g_acquire_semaphores[frame_index];
		VkResult result = vkAcquireNextImageKHR(device, vk_swapchain::g_swapchain, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, &image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			vk_swapchain::g_recreate_swapchain = true;
			return;
		}
		else if (result == VK_SUBOPTIMAL_KHR) {
			vk_swapchain::g_recreate_swapchain = true;
		}

		// begin recording commands
		VkCommandBufferBeginInfo buffer_begin_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		VkCommandBuffer& command_buffer = vk_sync::g_command_buffers[frame_index];
		vkBeginCommandBuffer(command_buffer, &buffer_begin_info);

		// transition image layout
		std::vector<VkImageMemoryBarrier2> layout_barriers{
			// swapchain image
			VkImageMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = VK_ACCESS_2_NONE,
				.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.image = vk_swapchain::g_swapchain_images[image_index],
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
				.image = vk_swapchain::g_depth_image,
				.subresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			},
		};
		VkDependencyInfo layout_dep_info{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = static_cast<uint32_t>(layout_barriers.size()),
			.pImageMemoryBarriers = layout_barriers.data(),
		};
		vkCmdPipelineBarrier2(vk_sync::g_command_buffers[frame_index], &layout_dep_info);

		VkRenderingAttachmentInfo color_attachment_info{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = vk_swapchain::g_swapchain_image_views[image_index],
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue{.color{0.0f, 0.0f, 0.0f, 0.0f}},
		};
		VkRenderingAttachmentInfo depth_attachment_info{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = vk_swapchain::g_depth_image_view,
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.clearValue{.depthStencil{1.0f, 0}},
		};

		VkRenderingInfo rendering_info{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea{
				.offset{.x = 0, .y = 0},
				.extent = vk_swapchain::g_swapchain_extent,
			},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &color_attachment_info,
			.pDepthAttachment = &depth_attachment_info,
		};

		vkCmdBeginRendering(command_buffer, &rendering_info);
		{
			VkViewport viewport{
				.x = 0,
				.y = 0,
				.width = static_cast<float>(vk_swapchain::g_swapchain_extent.width),
				.height = static_cast<float>(vk_swapchain::g_swapchain_extent.height),
			};
			vkCmdSetViewport(command_buffer, 0, 1, &viewport);

			VkRect2D scissor{
				.offset{.x = 0, .y = 0},
				.extent = vk_swapchain::g_swapchain_extent,
			};
			vkCmdSetScissor(command_buffer, 0, 1, &scissor);

			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline::g_pipeline);

			glm::mat4 model = glm::mat4(1.0f);
			glm::mat4 view = Orbit::GetViewMatrix();
			glm::mat4 projection = glm::perspective(glm::radians(Orbit::g_fov), (float)Window::GetWidth() / Window::GetHeight(), 0.1f, 100.0f);
			projection[1][1] *= -1;

			glm::mat4 mvp = projection * view * model;

			vkCmdPushConstants(command_buffer, vk_pipeline::g_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);

			vkCmdDraw(command_buffer, 36, 1, 0, 0);
		}
		vkCmdEndRendering(command_buffer);

		VkImageMemoryBarrier2 present_barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			.dstAccessMask = VK_ACCESS_2_NONE,
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.image = vk_swapchain::g_swapchain_images[image_index],
			.subresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		VkDependencyInfo present_dep_info{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &present_barrier,
		};
		vkCmdPipelineBarrier2(command_buffer, &present_dep_info);

		vkEndCommandBuffer(command_buffer);

		VkSemaphoreSubmitInfo acquire_wait_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = acquire_semaphore,
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		};
		std::vector<VkSemaphoreSubmitInfo> semaphore_signals{
			{   // binary semaphore
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = vk_sync::g_submit_semaphores[image_index],
				.value = 0,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
			},
			{   // timeline semaphore
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = vk_sync::g_timeline_semaphore,
				.value = signal_value,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			},
		};

		VkCommandBufferSubmitInfo buffer_submit_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = command_buffer,
		};
		VkSubmitInfo2 submit_info{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = 1,
			.pWaitSemaphoreInfos = &acquire_wait_info,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &buffer_submit_info,
			.signalSemaphoreInfoCount = static_cast<uint32_t>(semaphore_signals.size()),
			.pSignalSemaphoreInfos = semaphore_signals.data(),
		};
		const VkQueue& queue = vk_device::GetQueue();
		vkQueueSubmit2(queue, 1, &submit_info, VK_NULL_HANDLE);

		VkPresentInfoKHR present_info{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &vk_sync::g_submit_semaphores[image_index],
			.swapchainCount = 1,
			.pSwapchains = &vk_swapchain::g_swapchain,
			.pImageIndices = &image_index,
			.pResults = nullptr,
		};

		vkQueuePresentKHR(queue, &present_info);

		g_frame_count++;
		g_next_signal_value++;
	}

	void LoadShader(const std::string& filepath, const std::string& entry_point, const ShaderStage stage) {
		vk_pipeline::AddShader(filepath, entry_point, stage);
	}
}
