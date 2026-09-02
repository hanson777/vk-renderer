#pragma once
#include "Types/Vertex.h"
#include <vector>

namespace AssetManager {
    extern std::vector<Vertex> g_vertices;
    extern std::vector<uint32_t> g_indices;

    void LoadModels(const std::string& filename);
}
