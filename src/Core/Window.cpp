#include "Window.h"
#include <glfw/glfw3.h>
#include <cstdint>
#include <iostream>

namespace Window {

	GLFWwindow* g_handle = nullptr;
	uint32_t g_height = 0;
	uint32_t g_width = 0;

	void Init(uint32_t width, uint32_t height, const char* title) {
		if (!glfwInit()) {
			std::cout << "[ERROR::WINDOW] failed to initialize glfw" << std::endl;
			return;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		g_handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (!g_handle) {
			std::cout << "[ERROR::WINDOW] failed to create window" << std::endl;
			return;
		}

		glfwSetInputMode(g_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		g_height = height;
		g_width = width;
	}
	
	void BeginFrame() {
		glfwPollEvents();
	}

	bool ShouldClose() {
		return glfwWindowShouldClose(g_handle);
	}

	void Shutdown() {
		glfwTerminate();
	}

	const char** GetInstanceExtensions(uint32_t* extensionCount) {
		return glfwGetRequiredInstanceExtensions(extensionCount);
	}
}