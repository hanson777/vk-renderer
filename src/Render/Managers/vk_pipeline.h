#pragma once
#include "Render/vk_common.h"
#include "Render/Types/Shader.h"
#include <string>

struct Shader;

namespace vk_pipeline {

	extern VkPipelineLayout g_pipeline_layout;
	extern VkPipeline g_pipeline;

	bool Init();
	void Shutdown();
	void AddShader(const std::string& filepath, const std::string& entry_point, const ShaderStage stage);
}