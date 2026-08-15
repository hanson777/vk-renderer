#pragma once 
#include <glm/glm.hpp>

namespace Input {
   extern glm::vec2 g_current_position;
   extern glm::vec2 g_last_position;
   extern glm::vec2 g_start_position;

   extern float g_scroll_delta;
   extern bool g_dragging;
   extern bool g_mouse_moving;
    
   void Update();
   bool KeyPressed(int key);
   bool MousePressed(int button);
   void HandleMouseMove(float x_pos, float y_pos);
   void HandleMouseDrag(float x_pos, float y_pos, bool dragging);
   void HandleScroll(float dy);
}
