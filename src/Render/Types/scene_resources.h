#pragma once
#include "Render/Types/Tree.h"
#include "Render/Types/Vertex.h"
#include "Render/Types/Mesh.h"
#include "Render/Types/vk_buffer.h"
#include <cstdint>
#include <vector>

struct SceneResources {
    private:
    Tree tree;
    std::vector<Node> m_render_stack;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<Mesh> m_meshes;
    std::vector<Buffer> m_buffers;
    uint32_t m_vertex_buffer_id = UINT32_MAX;
    uint32_t m_index_buffer_id = UINT32_MAX;
    uint32_t m_material_buffer_id = UINT32_MAX;

    public:
    Tree& getTree() { return tree; }

    std::vector<Node>& getRenderStack() { return m_render_stack; }
    std::vector<Vertex>& getVertices() { return m_vertices; }
    std::vector<uint32_t>& getIndices() { return m_indices; }
    std::vector<Mesh>& getMeshes() { return m_meshes; }

    Buffer* getBuffer(uint32_t id);

    uint32_t getVertexBufferId() { return m_vertex_buffer_id; }
    uint32_t getIndexBufferId() { return m_index_buffer_id; }
    uint32_t getMaterialBufferId() { return m_material_buffer_id; }

    void setVertexBufferId(uint32_t id) { m_vertex_buffer_id = id; }
    void setIndexBufferId(uint32_t id) { m_index_buffer_id = id; }

    uint32_t addBuffer(Buffer buffer);
};
