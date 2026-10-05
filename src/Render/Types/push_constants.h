#pragma once
#include <glm/glm.hpp>

struct PushConstants {
    uint64_t scene_ref = 0;
    uint64_t vertex_ref = 0;
    uint64_t material_ref = 0;
    uint64_t instance_ref = 0;
};
