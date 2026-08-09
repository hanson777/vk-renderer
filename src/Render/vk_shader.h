#pragma once
#include "vk_common.h"
#include <string>

namespace shader {
	bool InitSlangSession();
}

struct VkShader {
	VkShader() = default;
	VkShader(const std::string& filename, const std::string& entryPointName, const VkShaderStageFlagBits stage);
	void LoadModule(const std::string& filename, const std::string& entryPointName);

	VkShaderModuleCreateInfo m_moduleCreateInfo{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	std::string m_entryPoint;
	VkShaderStageFlagBits m_stage = VK_SHADER_STAGE_ALL;
};

