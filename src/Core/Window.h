#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>

namespace Window {
	extern GLFWwindow* g_handle;
	extern uint32_t g_width;
	extern uint32_t g_height;

	void Init(uint32_t width, uint32_t height, const char* title);
	void BeginFrame();
	bool ShouldClose();
	void Shutdown();
	const char** GetInstanceExtensions(uint32_t* extensionCount);
};