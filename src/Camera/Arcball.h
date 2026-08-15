#pragma once
#include <glm/glm.hpp>

namespace Arcball {

    extern glm::vec3 g_position;
    extern float g_fov;

    void Init();
    void Update();
    glm::quat GetCurrentRotation();
    glm::mat4 GetViewMatrix();
};
