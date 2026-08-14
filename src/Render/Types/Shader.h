#pragma once
#include "Render/vk_common.h"
#include <slang.h>
#include <slang-com-ptr.h>
#include <string>

namespace slang_context {
	bool Init();
	bool InitSlangSession();
}

enum class ShaderStage {
	vertex = 0,
	fragment = 1,
};

struct Shader {
	Shader() = default;
	Shader(const std::string& filename, const std::string& entry_point_name, const ShaderStage stage);
	void LoadModule(const std::string& filename, const std::string& entry_point_name);

	VkShaderModuleCreateInfo m_module_create_info{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	std::string m_entry_point;
	const char* m_spirv_entry_point = "main";
	VkShaderStageFlagBits m_stage = VK_SHADER_STAGE_ALL;
	Slang::ComPtr<slang::IBlob> m_spirv;

private:
	void set_shader_stage(const ShaderStage stage);
};

