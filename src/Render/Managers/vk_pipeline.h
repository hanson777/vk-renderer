#pragma once
#include "Render/vk_common.h"

struct VkShader;

namespace vk_pipeline {

	extern VkPipeline g_pipeline;

	bool Init();
	void Shutdown();
	void AddShader(const VkShader shader);
}