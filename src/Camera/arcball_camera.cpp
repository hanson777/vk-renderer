#include "arcball_camera.h"
#include "Input/Input.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <algorithm>

namespace Arcball {
    
    glm::vec3 g_position{0.0f, 0.0f, 3.0f};

    glm::quat g_current_rotation;
    glm::quat g_last_rotation;

    float g_radius = 1.0f;
    float g_fov = 45.0f;

    glm::mat4 g_view = glm::lookAt(g_position, g_position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    static glm::quat compute_rotation_quat(glm::vec3 current_pos, glm::vec3 start_pos);
    static glm::vec3 project_to_sphere(glm::vec2 p);
    
    void Init() {
        g_current_rotation = glm::identity<glm::quat>();
        g_last_rotation = glm::identity<glm::quat>();
    }
    
    void Update() {
        if (Input::g_mouse_moving && Input::g_dragging) {
            glm::vec3 p1 = project_to_sphere(Input::g_current_position); 
            glm::vec3 p2 = project_to_sphere(Input::g_start_position);
            g_current_rotation = compute_rotation_quat(p2, p1);
        }
        if (!Input::g_dragging) {
            g_last_rotation = g_current_rotation * g_last_rotation;
            g_current_rotation = glm::identity<glm::quat>();
        }
        g_fov -= Input::g_scroll_delta;
        g_fov = std::clamp(g_fov, 1.0f, 45.0f);
    }

    glm::quat GetCurrentRotation() {
        return g_current_rotation * g_last_rotation;
    }

    glm::mat4 GetViewMatrix() { return g_view; }

    static glm::quat compute_rotation_quat(glm::vec3 current_pos, glm::vec3 start_pos) {
        return glm::rotation(current_pos, start_pos);
    }

    static glm::vec3 project_to_sphere(glm::vec2 p) {
        float d2 = p.x * p.x + p.y * p.y;
        float r2 = g_radius * g_radius;
        if (d2 <= r2) return glm::vec3(p, std::sqrt(r2 - d2));
        return glm::vec3(glm::normalize(p), 0.0f);
    }
}
