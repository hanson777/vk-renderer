#pragma once
#include "Render/Types/vk_buffer.h"
#include <glm/glm.hpp>
#include <array>

struct Scene {
    glm::mat4 mvp;
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> buffers;
};
