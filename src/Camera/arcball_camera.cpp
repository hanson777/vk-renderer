#include "arcball_camera.h"
#include "Input/Input.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cmath>
#include <algorithm>

namespace arcball_camera {
    
    glm::vec3 g_pivot{0.0f, 0.0f, -3.0f};
    glm::vec3 g_start_position;
    glm::vec3 g_current_position;

    glm::quat g_current_rotation;
    glm::quat g_last_rotation;

    static glm::quat compute_rotation_quat(glm::vec3 current, glm::vec3 start);
    static float z_axis(glm::vec2 point);
    
    void Init() {
        g_last_rotation = glm::identity<glm::quat>();
        g_current_rotation = glm::identity<glm::quat>();
    }
    
    void Update() {
        if (Input::g_mouse_moving) {
            glm::vec3 p1 = glm::vec3(Input::g_current_position, z_axis(Input::g_current_position));
            glm::vec3 p2 = glm::vec3(Input::g_start_position, z_axis(Input::g_start_position));
            g_current_rotation = compute_rotation_quat(p1, p2);
        }
        if (!Input::g_dragging) {
            g_last_rotation = g_current_rotation * g_last_rotation;
            g_current_rotation = glm::identity<glm::quat>();
        }
    }

    glm::quat GetCurrentRotation() {
        return g_current_rotation * g_last_rotation;
    }

    static glm::quat compute_rotation_quat(glm::vec3 p1, glm::vec3 p2) {
        float angle = std::acos(std::min(1.0f, glm::dot(p1, p2)));
        glm::vec3 axis = glm::normalize(glm::cross(p1, p2));

        if (glm::length(glm::cross(p1, p2)) < 1e-6f) {
            axis = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        return glm::angleAxis(angle, axis);
    }

    static float z_axis(glm::vec2 point) {
        float x2 = point.x * point.x;
        float y2 = point.y * point.y;
        if ((x2 + y2) <= 1) return std::sqrt(1.0f - x2 - y2);
        else return 0.0f;
    }
}
