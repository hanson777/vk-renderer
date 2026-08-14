#include "Application.h"
#include "Render/Renderer.h"
#include "Render/Types/Shader.h"

namespace Application {

	void Init() {
		Renderer::LoadShader("Shaders/triangle.slang", "vertMain", ShaderStage::vertex);
		Renderer::LoadShader("Shaders/triangle.slang", "fragMain", ShaderStage::fragment);
	}
}