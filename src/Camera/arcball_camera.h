#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace arcball_camera {
    void Init();
    glm::quat GetCurrentRotation();
};
