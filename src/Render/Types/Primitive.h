#pragma once
#include <cstdint>

struct Primitive {
    uint32_t index_count = 0;
    uint32_t first_index = 0;
    uint32_t vertex_count = 0;
    uint32_t vertex_start = 0;
    int32_t material_id = 0;
};
