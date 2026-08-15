#include "Window.h"
#include "Input/Input.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>

namespace Window {

	GLFWwindow* g_handle = nullptr;
	uint32_t g_height = 0;
	uint32_t g_width = 0;
    float g_aspect;
    
    void mouse_position_callback(GLFWwindow* window, double x_pos, double y_pos);
    void scroll_callback(GLFWwindow* window, double dx, double dy);
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

	bool Init(uint32_t width, uint32_t height, const char* title) {
		if (!glfwInit()) {
            std::cerr << "[ERROR::WINDOW] failed to initialize glfw\n";
            return false;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		g_handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (g_handle == nullptr) {
            std::cerr << "[ERROR::WINDOW] failed to create window\n";
			return false;
		}
        
        glfwSetCursorPosCallback(g_handle, mouse_position_callback);
        glfwSetScrollCallback(g_handle, scroll_callback);
        glfwSetMouseButtonCallback(g_handle, mouse_button_callback);

		g_height = height;
		g_width = width;
        g_aspect = static_cast<float>(width) / static_cast<float>(height);
        
        glfwSwapInterval(1);
        
        return true;
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
    
    GLFWwindow* GetWindowPointer() { return g_handle; }

    uint32_t GetWidth() { return g_width; }

    void SetWidth(uint32_t width) {
        g_width = width;
    }
    
    uint32_t GetHeight() { return g_height; }

    void SetHeight(uint32_t height) {
        g_height = height;
    }
    
    float GetAspect() { return g_aspect; }

	const char** GetInstanceExtensions(uint32_t* extension_count) {
		return glfwGetRequiredInstanceExtensions(extension_count);
	}

    void mouse_position_callback(GLFWwindow* window, double x_pos, double y_pos) {
        Input::HandleMouseMove(x_pos, y_pos);
    }
    
    void scroll_callback(GLFWwindow* window, double dx, double dy) {
        Input::HandleScroll(dy);
    } 
    
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double x_pos, y_pos;
            glfwGetCursorPos(window, &x_pos, &y_pos);
            Input::HandleMouseDrag(x_pos, y_pos, true);
        } else if (action == GLFW_RELEASE) {
            Input::HandleMouseDrag(0, 0, false);
        }
    }
}
