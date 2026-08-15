#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>

namespace Window {
	bool Init(uint32_t width, uint32_t height, const char* title);
	void BeginFrame();
	bool ShouldClose();
	void Shutdown();
    GLFWwindow* GetWindowPointer();
    uint32_t GetWidth();
    uint32_t GetHeight();
	void SetWidth(uint32_t width);
	void SetHeight(uint32_t height);
}
