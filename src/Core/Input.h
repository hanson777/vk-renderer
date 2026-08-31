#pragma once
#include <glm/glm.hpp>

namespace Input {
    extern glm::vec2 g_current_position;
    extern glm::vec2 g_last_position;
    extern glm::vec2 g_mouse_delta;

    extern float g_scroll_delta;

    extern bool g_dragging;
    extern bool g_mouse_moving;
    extern bool g_scrolling;

    void Update();
    bool KeyPressed(int key);
    void HandleMouseMove(float x_pos, float y_pos);
    void HandleMouseDrag(bool dragging);
    void HandleScroll(float dy);
}