#pragma once
#include "Render/vk_common.h"
#include "Render/Types/Tree.h"
#include "Render/Types/Vertex.h"
#include "Render/Types/vk_image.h"
#include "Render/Types/Texture.h"
#include "Render/Types/Material.h"
#include "Render/Types/Mesh.h"
#include "Render/Types/vk_buffer.h"
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>

struct Instance {
    Node* node = nullptr;
    glm::mat4 matrix = glm::mat4(1.0f);
};

struct SceneResources {
    private:
    Tree tree;
    std::vector<Instance> m_render_stack;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<uint8_t> m_image_buffer;
    std::vector<Image> m_images;
    std::vector<VkSampler> m_samplers;
    std::vector<Texture> m_textures;
    std::vector<Material> m_materials;
    std::vector<Mesh> m_meshes;
    std::vector<Buffer> m_buffers;
    uint32_t m_vertex_buffer_id = UINT32_MAX;
    uint32_t m_index_buffer_id = UINT32_MAX;
    uint32_t m_material_buffer_id = UINT32_MAX;
    uint32_t m_fallback_image_id = UINT32_MAX;
    uint32_t m_fallback_sampler_id = UINT32_MAX;
    uint32_t m_fallback_texture_id = UINT32_MAX;
    uint32_t m_fallback_material_id = UINT32_MAX;

    public:
    void shutdown();

    Tree& getTree() { return tree; }

    std::vector<Instance>& getRenderStack() { return m_render_stack; }
    std::vector<Vertex>& getVertices() { return m_vertices; }
    std::vector<uint32_t>& getIndices() { return m_indices; }
    std::vector<uint8_t>& getImageBuffer() { return m_image_buffer; }
    std::vector<Image>& getImages() { return m_images; }
    std::vector<VkSampler>& getSamplers() { return m_samplers; }
    std::vector<Texture>& getTextures() { return m_textures; }
    std::vector<Material>& getMaterials() { return m_materials; }
    std::vector<Mesh>& getMeshes() { return m_meshes; }

    uint32_t addBuffer(Buffer buffer);
    Buffer* getBuffer(uint32_t id);
    void destroyBuffers() { m_buffers.clear(); }

    void destroyImages();
    void destroySamplers();

    uint32_t getVertexBufferId() { return m_vertex_buffer_id; }
    uint32_t getIndexBufferId() { return m_index_buffer_id; }
    uint32_t getMaterialBufferId() { return m_material_buffer_id; }

    uint32_t getFallbackImageId() { return m_fallback_image_id; }
    uint32_t getFallbackSamplerId() { return m_fallback_sampler_id; }
    uint32_t getFallbackTextureId() { return m_fallback_texture_id; }
    uint32_t getFallbackMaterialId() { return m_fallback_material_id; }

    void setVertexBufferId(uint32_t id) { m_vertex_buffer_id = id; }
    void setIndexBufferId(uint32_t id) { m_index_buffer_id = id; }
    void setMaterialBufferId(uint32_t id) { m_material_buffer_id = id; }

    void setFallbackImageId(uint32_t id) { m_fallback_image_id = id; }
    void setFallbackSamplerId(uint32_t id) { m_fallback_sampler_id = id; }
    void setFallbackTextureId(uint32_t id) { m_fallback_texture_id = id; }
    void setFallbackMaterialId(uint32_t id) { m_fallback_material_id = id; }
};
