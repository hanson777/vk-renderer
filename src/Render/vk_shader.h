#pragma once
#include "vk_common.h"
#include <slang/slang.h>
#include <string>

namespace Shader {
	bool InitSlangSession();
}

struct Shader {
	bool Init();

	VkShaderModule module = VK_NULL_HANDLE;
	std::string filename;
};

