#include "Orbit.h"
#include "Input/Input.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <cmath>

namespace Orbit {
    glm::vec3 g_position(0.0f, 0.0f, 3.0f);
    glm::vec3 g_front(0.0f, 0.0f, -1.0f);
    glm::vec3 g_up(0.0f, 1.0f, 0.0f);

    float g_fov = 45.0f;
    float g_radius = 5.0f;
    float g_theta = std::acos(glm::normalize(g_position).y / g_radius);
    float g_phi = std::acos(glm::normalize(g_position).z / g_radius);

    void Update() {
        if (Input::g_dragging && Input::g_mouse_moving) {
            
        }

        g_fov = std::clamp(g_fov - Input::g_scroll_delta, 1.0f, 45.0f);
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(g_position, g_position + g_front, g_up);
    }

}