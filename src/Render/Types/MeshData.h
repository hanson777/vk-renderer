#pragma once
#include "Render/Types/vk_buffer.h"

struct MeshData {
    Buffer vertex_buffer;
    uint64_t vertex_bda;
    Buffer index_buffer;
    uint64_t index_bda;
    uint32_t index_count;
};
