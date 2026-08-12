#include "Application.h"
#include "Render/renderer.h"
#include "Render/shader.h"

namespace Application {

	void Init() {
		Renderer::LoadShader("Shaders/triangle.slang", "vertMain", ShaderStage::vertex);
		Renderer::LoadShader("Shaders/triangle.slang", "fragMain", ShaderStage::fragment);
	}
}