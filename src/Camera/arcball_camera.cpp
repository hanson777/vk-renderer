#include "arcball_camera.h"
#include "Input/Input.h"
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace arcball_camera {
    
    glm::vec3 g_pivot{0.0f, 0.0f, -3.0f};
    glm::vec3 g_start_position;
    glm::vec3 g_current_position;

    glm::quat g_current_rotation;
    glm::quat g_last_rotation;
    
    bool g_dragging = false;
    
    static glm::quat compute_rotation_quat(glm::vec3 current, glm::vec3 start);
    static float z_axis(float x, float y);
    
    void Init() {
       g_last_rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f); 
       g_current_rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f); 
    }
    
    void Update() {
        if (Input::g_mouse_moving) {
        }
    }

    static glm::quat compute_rotation_quat(glm::vec3 current, glm::vec3 start) {
        
    }

    static float z_axis(float x, float y) {
        float x2 = x*x;
        float y2 = y*y;
        if ((x2 + y2) <= 1) return std::sqrt(1 - x2 - y2);
        else return 0.0f;
    }
}
