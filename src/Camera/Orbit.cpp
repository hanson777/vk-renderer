#include "Orbit.h"
#include "Input/Input.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace Orbit {
    float g_fov = 45.0f;
    float g_radius = 1.0f;
    float g_distance = 3.0f;
    float g_pitch_limit = 89.5f;
    float g_pitch = 45.0f;
    float g_yaw = -45.0f;
    float g_sensitivity = 0.15f;

    glm::vec3 g_position = glm::vec3(
        cos(glm::radians(g_pitch)) * cos(glm::radians(g_yaw)),
        sin(glm::radians(g_pitch)),
        cos(glm::radians(g_pitch)) * sin(glm::radians(g_yaw))
    ) * g_radius * g_distance;

    static void compute_position();

    void Update() {
        bool dirty = false;

        if (Input::g_scrolling) {
            g_distance = std::clamp(g_distance - Input::g_scroll_delta * g_sensitivity, g_radius + 1.0f, 10.0f);
            dirty = true;
        }

        if (Input::g_dragging && Input::g_mouse_moving) {
            g_yaw += (Input::g_mouse_delta.x * g_sensitivity);
            g_pitch += (Input::g_mouse_delta.y * g_sensitivity);
            g_pitch = std::clamp(g_pitch, -g_pitch_limit, g_pitch_limit);
            dirty = true;
        }

        if (dirty) compute_position();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAtRH(g_position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    static void compute_position() {
        float theta = glm::radians(g_pitch);
        float phi = glm::radians(g_yaw);

        g_position.x = cos(theta) * cos(phi);
        g_position.y = sin(theta);
        g_position.z = cos(theta) * sin(phi);
        g_position *= g_radius;
        g_position *= g_distance;
    }
}