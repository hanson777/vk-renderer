#pragma once
#include "Render/Types/Shader.h"
#include <string>

namespace Renderer {
	extern uint64_t g_next_signal_value;

	void Render();
    void Shutdown();
    void PrepareUniformBuffers();
	void LoadShader(const std::string& filepath, const std::string& entry_point, const ShaderStage stage);
}
