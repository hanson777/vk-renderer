#pragma once
#include "Render/Types/Shader.h"
#include <string>

struct FpsCamera;

namespace Renderer {
	extern uint64_t g_next_signal_value;

	void Render(const FpsCamera& camera);
    void Shutdown();
    bool PrepareSceneResources(const std::string& filename);
	void LoadShader(const std::string& filepath, const std::string& entry_point, const ShaderStage stage);
}
