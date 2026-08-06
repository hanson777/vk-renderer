#include "vk_pipeline.h"
#include "Render/vk_common.h"
#include "vk_device.h"
#include "Render/vk_shader.h"
#include <iostream>
#include <vector>

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

		if (vkCreatePipelineLayout(VkDeviceManager::GetLogicalDevice(), &pipelineLayoutCreateInfo, nullptr, &g_pipelineLayout) != VK_SUCCESS) {
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
	}

	void AddShader(const VkShader& shader) {
		g_shaders.push_back(shader);
	}
}