#include "arcball_camera.h"
#include "Input/Input.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <algorithm>

namespace Arcball {

    glm::vec3 g_position{0.0f, 0.0f, 3.0f};

    glm::quat g_rotation;

    float g_radius = 1.0f;
    float g_fov = 45.0f;

    glm::mat4 g_view = glm::lookAt(g_position, g_position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    static glm::vec3 project_to_sphere(glm::vec2 p);

    void Init() {
        g_rotation = glm::identity<glm::quat>();
    }

    void Update() {
        if (Input::g_dragging && Input::g_mouse_moving) {
            glm::vec3 start = project_to_sphere(Input::g_last_position);
            glm::vec3 dest = project_to_sphere(Input::g_current_position);
            g_rotation = glm::normalize(glm::rotation(start, dest) * g_rotation);
        }

        g_fov = std::clamp(g_fov - Input::g_scroll_delta, 1.0f, 45.0f);
    }

    glm::quat GetCurrentRotation() {
        return g_rotation;
    }

    glm::mat4 GetViewMatrix() { return g_view; }

    static glm::vec3 project_to_sphere(glm::vec2 p) {
        float d2 = p.x * p.x + p.y * p.y;
        float r2 = g_radius * g_radius;
        float z = (d2 <= r2 * 0.5f) ? std::sqrt(r2 - d2) : (r2 * 0.5f) / std::sqrt(d2);
        return glm::normalize(glm::vec3(p, z));
    }
}
