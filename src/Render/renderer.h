#pragma once
#include "shader.h"
#include <string>

namespace Renderer {
	void Render();
	void LoadShader(const std::string& filepath, const std::string& entryPoint, const ShaderStage stage);
}