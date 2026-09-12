#pragma once
#include "Render/vk_common.h"
#include "Render/Types/vk_buffer.h"
#include "Render/Types/Node.h"
#include "Types/Vertex.h"
#include <tiny_gltf_v3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <cstddef>

struct Primitive {
    uint32_t index_count = 0;
    uint32_t first_index = 0;
    uint32_t vertex_count = 0;
    uint32_t vertex_start = 0;
    int32_t material_id = 0;
};

struct Mesh {
    std::vector<Primitive> primitives;
};

struct Attribute {
    size_t offset;
    size_t stride;
    const std::byte* start;
    int num_floats;
};

struct Model {
    Tree m_nodeTree;
    std::vector<Buffer> m_buffers;
    std::vector<Mesh> m_meshes;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    uint32_t m_vert_buffer_id = 0;
    uint32_t m_index_buffer_id = 0;
    uint32_t m_vert_offset;
    uint32_t m_index_offset;

    private:
    void loadGltf(std::string& filename);
    void loadMeshes(const tg3_model& model);
};
