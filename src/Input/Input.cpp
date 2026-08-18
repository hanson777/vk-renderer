#include "Input.h"
#include "Core/Window.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Input {

    glm::vec2 g_current_position(0.0f);
    glm::vec2 g_last_position(0.0f);

    float g_scroll_delta = 0.0f;

    bool g_dragging = false;
    bool g_mouse_moving = false;

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
            glm::vec2 point{x_pos, y_pos};
            g_last_position = point;
            g_current_position = point;
            g_dragging = true;
        } else {
            g_dragging = false;
        }
    }

    void HandleMouseMove(float x_pos, float y_pos) {
        g_mouse_moving = true;
        g_current_position = {x_pos, y_pos};
    }

    void HandleScroll(float dy) {
        g_scroll_delta = dy;
    }
}