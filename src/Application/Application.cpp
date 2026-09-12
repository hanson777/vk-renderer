#include "Application.h"
#include "Render/Renderer.h"
#include "Render/Types/Shader.h"

namespace Application {

	void Init() {
		Renderer::LoadShader("src/Shaders/cube.slang", "vertMain", ShaderStage::vertex);
		Renderer::LoadShader("src/Shaders/cube.slang", "fragMain", ShaderStage::fragment);
	}
}
