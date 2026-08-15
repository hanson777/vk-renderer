#include "arcball_camera.h"
#include "Input/Input.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace Arcball {
    
    glm::vec3 g_position{0.0f, 0.0f, 3.0f};

    glm::quat g_current_rotation;
    glm::quat g_last_rotation;

    float g_radius = 1.0f;
    float g_fov = 45.0f;

    glm::mat4 g_view = glm::lookAt(g_position, g_position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

    static glm::quat compute_rotation_quat(glm::vec3 current, glm::vec3 start);
    static float z_axis(glm::vec2 point);
    
    void Init() {
        g_current_rotation = glm::identity<glm::quat>();
        g_last_rotation = glm::identity<glm::quat>();
    }
    
    void Update() {
        if (Input::g_mouse_moving && Input::g_dragging) {
            glm::vec3 p1 = glm::vec3(Input::g_current_position, z_axis(Input::g_current_position));
            glm::vec3 p2 = glm::vec3(Input::g_start_position, z_axis(Input::g_start_position));
            g_current_rotation = compute_rotation_quat(p1, p2);
        }
        if (!Input::g_dragging) {
            g_last_rotation = g_current_rotation * g_last_rotation;
            g_current_rotation = glm::identity<glm::quat>();
        }
        g_fov -= Input::g_scroll_delta;
    }

    glm::quat GetCurrentRotation() {
        return g_current_rotation * g_last_rotation;
    }

    glm::mat4 GetViewMatrix() { return g_view; }

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
        if ((x2 + y2) <= 1) return std::sqrt(g_radius - x2 - y2);
        else return 0.0f;
    }
}
