#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

int main() {
	if (!glfwInit()) {
		std::cout << "failed to init glfw\n";
		return 1;
	}
}