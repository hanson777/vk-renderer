#include "vk_pipeline.h"
#include "Render/vk_common.h"
#include "vk_device.h"
#include "Render/shader.h"
#include "vk_swapchain.h"
#include <iostream>
#include <vector>
#include <cstdint>

namespace vk_pipeline {
	
	VkPipelineLayout g_pipeline_layout = VK_NULL_HANDLE;
	VkPipeline g_pipeline = VK_NULL_HANDLE;
	std::vector<Shader> g_shaders;
	std::vector<VkPipelineShaderStageCreateInfo> g_shader_stage_create_infos;

	bool Init() {
		VkPipelineLayoutCreateInfo pipeline_layout_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 0,
			.pushConstantRangeCount = 0,
		};

		const VkDevice& device = vk_device::GetDevice();
		if (vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &g_pipeline_layout) != VK_SUCCESS) {
			std::cerr << "[ERROR::PIPELINE_MANAGER] failed to create pipeline layout\n";
			return false;
		}
		
		for (int i = 0; i < g_shaders.size(); i++) {
			std::cout << "shader stage: " << g_shaders[i].m_stage << '\n';
			VkPipelineShaderStageCreateInfo ss_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = &g_shaders[i].m_module_create_info,
				.stage = g_shaders[i].m_stage,
				.pName = g_shaders[i].m_spirv_entry_point,
			};
			g_shader_stage_create_infos.push_back(ss_create_info);
		}

		VkPipelineVertexInputStateCreateInfo vertex_input_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		};

		VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		};

		VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.stencilTestEnable = VK_FALSE,
		};

		VkPipelineViewportStateCreateInfo viewport_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = nullptr,
			.scissorCount = 1,
			.pScissors = nullptr,
		};

		VkPipelineRasterizationStateCreateInfo raster_state_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.lineWidth = 1.,
		};

		VkPipelineMultisampleStateCreateInfo multisample_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		};

		VkPipelineColorBlendAttachmentState blend_attachment_state{
			.blendEnable = VK_FALSE,
			.colorWriteMask = 0xF,
		};	

		VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = &blend_attachment_state,
		};

		std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamic_state_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
			.pDynamicStates = dynamic_states.data(),
		};

		VkPipelineRenderingCreateInfo rendering_create_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &vk_swapchain::g_swapchain_image_format,
			.depthAttachmentFormat = vk_swapchain::g_depth_image_format,
		};

		VkGraphicsPipelineCreateInfo pipeline_create_info{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &rendering_create_info,
			.stageCount = static_cast<uint32_t>(g_shader_stage_create_infos.size()),
			.pStages = g_shader_stage_create_infos.data(),
			.pVertexInputState = &vertex_input_create_info,
			.pInputAssemblyState = &input_assembly_create_info,
			.pViewportState = &viewport_create_info,
			.pRasterizationState = &raster_state_create_info,
			.pMultisampleState = &multisample_create_info,
			.pDepthStencilState = &depth_stencil_create_info,
			.pColorBlendState = &color_blend_state_create_info,
			.pDynamicState = &dynamic_state_create_info,
			.layout = g_pipeline_layout,
			.renderPass = VK_NULL_HANDLE,
		};

		if (vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_create_info, nullptr, &g_pipeline) != VK_SUCCESS) {
			std::cerr << "[ERROR::PIPELINE_MANAGER] failed to create graphics pipeline\n";
			return false;
		}

		return true;
	}

	void Shutdown() {
		const VkDevice device = vk_device::GetDevice();
		if (g_pipeline_layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(device, g_pipeline_layout, nullptr);
		}

		if (g_pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(device, g_pipeline, nullptr);
		}
	}

	void AddShader(const std::string& filepath, const std::string& entry_point, const ShaderStage stage) {
		g_shaders.emplace_back(filepath, entry_point, stage);
	}
}