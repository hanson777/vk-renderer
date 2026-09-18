#pragma once
#include <glm/glm.hpp>
#include <cstdint>

struct Material {
    glm::vec4 base_color = glm::vec4(1.0f);
    uint32_t texture_index = 0;
};
