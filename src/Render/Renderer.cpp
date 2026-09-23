#include "Renderer.h"
#include "Render/vk_common.h"
#include "Render/Managers/vk_device.h"
#include "Render/Managers/vk_swapchain.h"
#include "Render/Managers/vk_pipeline.h"
#include "Render/Managers/vk_sync.h"
#include "Render/Types/Shader.h"
#include "Render/Types/vk_buffer.h"
#include "Render/Types/Node.h"
#include "Render/Types/scene_resources.h"
#include "Render/Types/gltf_loader.h"
#include "Render/Types/push_constants.h"
#include "Camera/Orbit.h"
#include "Core/Window.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <cstdint>
#include <vector>
#include <array>
#include <string>

namespace Renderer {

	uint32_t g_frame_count = 0;
	uint64_t g_next_signal_value = vk_sync::g_timeline_value + 1;

    struct {
        glm::mat4 mvp;
        std::array<Buffer, MAX_FRAMES_IN_FLIGHT> buffers;
    } scene;
    SceneResources scene_resources;
    GltfLoader gltf_loader;

    void Shutdown() {
        vkDeviceWaitIdle(vk_device::GetDevice());
        for (Buffer& buffer : scene.buffers) {
            buffer.flush(sizeof(glm::mat4), 0);
            buffer.unmap();
            buffer = {};
        }
        scene_resources.shutdown();
    }

    void PrepareUniformBuffers() {
        // gltf_loader.loadGltf("/Users/hanson/graphics/vk-renderer/res/ABeautifulGame.glb", scene_resources);
        // gltf_loader.loadGltf("/Users/hanson/Downloads/revolver_navy_colt_1851_silver/scene.gltf", scene_resources);
        gltf_loader.loadGltf("/Users/hanson/graphics/vk-renderer/res/MosquitoInAmber.glb", scene_resources);
        Node* root = scene_resources.getTree().getNode(scene_resources.getTree().m_root_node_id);
        root->setTranslation(glm::vec3(0, 0, 0));
        root->setRotation(glm::quat(1, 0, 0, 0));
        root->setScale(glm::vec3(10, 10, 10));

        for (uint32_t i = 0; i < scene.buffers.size(); i++) {
            scene.buffers[i] = createBuffer(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, sizeof(glm::mat4), true, VMA_MEMORY_USAGE_AUTO);
            scene.buffers[i].map();
        }

        std::vector<GpuInstance> gpu_instances;
        std::vector<VkDrawIndexedIndirectCommand> draw_commands;
        std::vector<std::pair<uint32_t, glm::mat4>> node_stack;

        node_stack.push_back({
            scene_resources.getTree().m_root_node_id,
            glm::mat4(1.0f),
        });

        while (!node_stack.empty()) {
            const auto [node_id, parent_world] = node_stack.back();
            node_stack.pop_back();

            Node* node = scene_resources.getTree().getNode(node_id);
            glm::mat4 world_matrix = parent_world * node->getMatrix();

            if (node->getMeshId() != UINT32_MAX) {
                Mesh& mesh = scene_resources.getMeshes()[node->getMeshId()];
                for (Primitive& primitive : mesh.primitives) {
                    uint32_t instance_index = static_cast<uint32_t>(gpu_instances.size());
                    gpu_instances.push_back({ world_matrix, static_cast<uint32_t>(primitive.material_id) });

                    draw_commands.push_back({
                        .indexCount = primitive.index_count,
                        .instanceCount = 1,
                        .firstIndex = primitive.first_index,
                        .vertexOffset = static_cast<int32_t>(primitive.vertex_start),
                        .firstInstance = instance_index,
                    });
                }
            }

            for (uint32_t child_id = node->getFirstChildId(); child_id != UINT32_MAX; ) {
                Node* child = scene_resources.getTree().getNode(child_id);
                uint32_t next_sibling_id = child->getNextSiblingId();
                node_stack.push_back({ child_id, world_matrix });
                child_id = next_sibling_id;
            }
        }

        size_t instance_buf_size = sizeof(gpu_instances[0]) * gpu_instances.size();
        size_t draw_cmds_buf_size = sizeof(draw_commands[0]) * draw_commands.size();

        Buffer instance_staging = createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, instance_buf_size, true, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
        Buffer draw_cmds_staging = createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, draw_cmds_buf_size, true, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
        instance_staging.map();
        draw_cmds_staging.map();

        memcpy(instance_staging.mapped, gpu_instances.data(), instance_buf_size);
        memcpy(draw_cmds_staging.mapped, draw_commands.data(), draw_cmds_buf_size);

        instance_staging.flush(instance_buf_size, 0);
        instance_staging.unmap();
        draw_cmds_staging.flush(draw_cmds_buf_size, 0);
        draw_cmds_staging.unmap();

        Buffer instance_buffer = createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, instance_buf_size, false, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        Buffer draw_cmds_buffer = createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT , draw_cmds_buf_size, false, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        copyBuffer(instance_staging, instance_buffer, instance_buf_size);
        copyBuffer(draw_cmds_staging, draw_cmds_buffer, draw_cmds_buf_size);
    }

    void UpdateUniformBuffers(int frame_index) {
        glm::mat4 projection = glm::perspective(glm::radians(Orbit::g_fov), (float)Window::GetWidth() / Window::GetHeight(), 0.1f, 100.0f);
        projection[1][1] *= -1;
        scene.mvp = projection * Orbit::GetViewMatrix();
        memcpy(scene.buffers[frame_index].mapped, &scene.mvp, sizeof(glm::mat4));
        scene.buffers[frame_index].flush(sizeof(glm::mat4), 0);
    }

	void Render() {
		const VkDevice& device = vk_device::GetDevice();

		if (vk_swapchain::g_recreate_swapchain) {
			vkDeviceWaitIdle(device); 
			vk_swapchain::RecreateSwapchain();
			vk_swapchain::g_recreate_swapchain = false;
		}

		const uint32_t frame_index = g_frame_count % MAX_FRAMES_IN_FLIGHT;
		const uint64_t signal_value = g_next_signal_value;
		const uint64_t wait_value = signal_value - MAX_FRAMES_IN_FLIGHT;

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
		VK_CHECK(vkBeginCommandBuffer(command_buffer, &buffer_begin_info));

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

            Buffer* vertex_buffer = scene_resources.getBuffer(scene_resources.getVertexBufferId()); 
            Buffer* index_buffer = scene_resources.getBuffer(scene_resources.getIndexBufferId()); 

            vkCmdBindIndexBuffer(command_buffer, index_buffer->buffer, 0, VK_INDEX_TYPE_UINT32);

            UpdateUniformBuffers(frame_index);

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
		VK_CHECK(vkQueueSubmit2(queue, 1, &submit_info, VK_NULL_HANDLE));

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
