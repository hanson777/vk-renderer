#pragma once
#include <glm/glm.hpp>

namespace Orbit {
    extern float g_fov;

    void Update();
    glm::mat4 GetViewMatrix();
}