#include "Input.h"
#include "Core/Window.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Input {
    glm::vec2 g_current_position(0.0f);
    glm::vec2 g_last_position(0.0f);
    glm::vec2 g_mouse_delta(0.0f);

    float g_scroll_delta = 0.0f;

    bool g_first_mouse = true;
    bool g_dragging = false;
    bool g_mouse_moving = false;
    bool g_scrolling = false;

    void Update() {
        g_last_position = g_current_position;
        g_mouse_delta = { 0.0f, 0.0f };
        g_scroll_delta = 0.0f;
        g_mouse_moving = false;
        g_scrolling = false;
    }

    bool KeyPressed(int key) {
        return glfwGetKey(Window::GetHandle(), key) == GLFW_PRESS;
    }

    void HandleMouseDrag(bool dragging) {
        g_dragging = dragging;
    }

    void HandleMouseMove(float x_pos, float y_pos) {
        if (g_first_mouse) {
            g_last_position = { x_pos, y_pos };
            g_current_position = { x_pos, y_pos };
            g_first_mouse = false;
        }
        g_mouse_delta.x += x_pos - g_last_position.x;
        g_mouse_delta.y += g_last_position.y - y_pos;
        g_current_position = { x_pos, y_pos };
        g_mouse_moving = true;
    }

    void HandleScroll(float dy) {
        g_scrolling = true;
        g_scroll_delta = dy;
    }
}