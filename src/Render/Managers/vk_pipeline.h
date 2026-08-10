#pragma once
#include "vk_common.h"

namespace vk_pipeline {

	extern VkPipeline g_pipeline;

	bool Init();
	void Shutdown();
	void AddShader();
}