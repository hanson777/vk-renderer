#include "Input.h"
#include "Core/Window.h"
#include <glm/glm.hpp>
#include <algorithm>

namespace Input {

   glm::vec2 g_current_position(0.0f);
   glm::vec2 g_last_position(0.0f);

   float g_scroll_delta = 0.0f;
   bool g_dragging = false;
   bool g_mouse_moving = false;
    
   static glm::vec2 ndc(float x, float y);

   void Update() {
        g_last_position = g_current_position;
        g_scroll_delta = 0.0f;
        g_mouse_moving = false;
    }

    bool KeyPressed(int key) {
         return glfwGetKey(Window::GetWindowPointer(), key) == GLFW_PRESS;
    }

    void HandleMouseDrag(float x_pos, float y_pos, bool dragging) {
        if (dragging) {
            glm::vec2 point = ndc(x_pos, y_pos);
            g_last_position = point;
            g_current_position = point;
            g_dragging = true;
        } else {
            g_dragging = false;
        }
    }

    void HandleMouseMove(float x_pos, float y_pos) {
        g_mouse_moving = true;
        g_current_position = ndc(x_pos, y_pos); 
    }
    
    void HandleScroll(float dy) {
        g_scroll_delta = dy;
    }

    static glm::vec2 ndc(float x, float y) {
        float width = static_cast<float>(Window::GetWidth());
        float height = static_cast<float>(Window::GetHeight());
        float scale = 2.0f / std::min(width, height);
        return glm::vec2(x - width * 0.5f, height * 0.5f - y) * scale;
    }
}
