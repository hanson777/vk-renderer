#include "Input.h"
#include "Core/Window.h"
#include <glm/glm.hpp>
#include <iostream>

namespace Input {

   glm::vec2 g_current_position(Window::GetHeight() / 2, Window::GetWidth() / 2);
   glm::vec2 g_last_position(0.0f);
   glm::vec2 g_start_position(0.0f);
   glm::vec2 g_mouse_delta(0.0f); 

   float g_scroll_delta = 0.0f;
   bool g_dragging = false;
   bool g_mouse_moving = false;

   void Update() {
        g_mouse_delta = glm::vec2(0.0f, 0.0f); 
        g_scroll_delta = 0.0f;
        g_mouse_moving = false;
    }

    bool KeyPressed(int key) {
         return glfwGetKey(Window::GetWindowPointer(), key) == GLFW_PRESS;
    }
    
    void HandleMouseDrag(float x_pos, float y_pos, bool dragging) {
        if (dragging) {
            std::cout << "You are dragging the mouse.\n";
            g_mouse_delta = glm::vec2(x_pos, y_pos) - g_last_position; 
            g_last_position = glm::vec2(x_pos, y_pos);
            g_dragging = true;
        } else {
            std::cout << "You've stopped dragging the mouse.\n";
            g_dragging = false;
        }
    }

    void HandleMouseMove(float x_pos, float y_pos) {
        g_mouse_moving = true;
        g_current_position = glm::vec2(x_pos, y_pos); 
    }
    
    void HandleScroll(float dy) {
        g_scroll_delta = dy;
    }
}
