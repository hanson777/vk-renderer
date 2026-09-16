#pragma once
#include <glm/glm.hpp>

struct PushConstants {
    uint64_t scene_ref = 0;
    uint64_t vertex_ref = 0;
    glm::mat4 model = glm::mat4(1.0f);
};
