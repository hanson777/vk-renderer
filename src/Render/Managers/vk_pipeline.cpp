#include "vk_pipeline.h"
#include "Render/vk_common.h"
#include "vk_device.h"
#include "Render/vk_shader.h"
#include "vk_swapchain.h"
#include <iostream>
#include <vector>
#include <cstdint>

namespace VkPipelineManager {
	
	VkPipelineLayout g_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline g_pipeline = VK_NULL_HANDLE;
	std::vector<VkShader> g_shaders;

	bool Init() {
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 0,
			.pushConstantRangeCount = 0,
		};

		const VkDevice device = VkDeviceManager::GetLogicalDevice();
		if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &g_pipelineLayout) != VK_SUCCESS) {
			std::cerr << "[ERROR::PIPELINE_MANAGER] failed to create pipeline layout\n";
			return false;
		}
		
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos(g_shaders.size(), { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO });
		for (const VkShader& shader : g_shaders) {
			VkPipelineShaderStageCreateInfo ssCreateInfo{
				.pNext = &shader.m_moduleCreateInfo,
				.stage = shader.m_stage,
				.pName = shader.m_entryPoint.c_str(),
			};
			shaderStageCreateInfos.push_back(ssCreateInfo);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		};

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		};

		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.stencilTestEnable = VK_FALSE,
		};

		VkPipelineViewportStateCreateInfo viewportCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = nullptr,
			.scissorCount = 1,
			.pScissors = nullptr,
		};

		VkPipelineRasterizationStateCreateInfo rasterStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.lineWidth = 1.,
		};

		VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		};

		VkPipelineColorBlendAttachmentState blendAttachmentState{
			.blendEnable = VK_FALSE,
			.colorWriteMask = 0xF,
		};	

		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = &blendAttachmentState,
		};

		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data(),
		};

		VkPipelineRenderingCreateInfo renderingCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = VkSwapchainManager::GetSwapchainImageFormat(),
			.depthAttachmentFormat = VkSwapchainManager::GetDepthImageFormat(),
		};

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &renderingCreateInfo,
			.stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size()),
			.pStages = shaderStageCreateInfos.data(),
			.pVertexInputState = &vertexInputCreateInfo,
			.pInputAssemblyState = &inputAssemblyCreateInfo,
			.pViewportState = &viewportCreateInfo,
			.pRasterizationState = &rasterStateCreateInfo,
			.pMultisampleState = &multisampleCreateInfo,
			.pDepthStencilState = &depthStencilCreateInfo,
			.pColorBlendState = &colorBlendStateCreateInfo,
			.pDynamicState = &dynamicStateCreateInfo,
			.layout = g_pipelineLayout,
			.renderPass = VK_NULL_HANDLE,
		};

		if (vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &g_pipeline) != VK_SUCCESS) {
			std::cerr << "[ERROR::PIPELINE_MANAGER] failed to create graphics pipeline\n";
			return false;
		}

		return true;
	}

	void AddShader(const VkShader& shader) {
		g_shaders.push_back(shader);
	}
}