#include "gltf_loader.h"
#include "Render/Types/Vertex.h"
#include <tiny_gltf_v3.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cstdint>
#include <iostream>

void GltfLoader::loadGltf(const std::string& filename, SceneResources& scene_resources) {
    tg3_parse_options opts;
    tg3_error_stack errors;
    tg3_model model;

    tg3_parse_options_init(&opts);
    tg3_error_stack_init(&errors);

    tg3_error_code err = tg3_parse_file(&model, &errors, filename.c_str(), filename.length(), &opts);
    if (err != TG3_OK) {
        for (uint32_t i = 0; i < errors.count; i++) {
            std::cerr << "[ERROR::loadModels] " << "Severity: " << errors.entries[i].severity << ' ' <<
                    "Message: " << (errors.entries[i].message ? errors.entries[i].message : "(null)") << '\n';
        }
    }

    std::vector<uint32_t> mesh_ids = loadMeshes(model,  scene_resources);

    const tg3_scene* scene = &model.scenes[model.default_scene != -1 ? model.default_scene : 0];

    Tree& tree = scene_resources.getTree();
    for (uint32_t i = 0; i < scene->nodes_count; i++) {
        uint32_t node_id = importNode(tree, model, scene->nodes[i], 0, tree.m_last_root_node_id, mesh_ids);
        if (tree.m_root_node_id == UINT32_MAX) {
            tree.m_root_node_id = node_id;
            tree.m_last_root_node_id = node_id;
        }
        else {
            tree.m_last_root_node_id = node_id;
        }
    }

    tg3_model_free(&model);
    tg3_error_stack_free(&errors);
}

std::vector<uint32_t> GltfLoader::loadMeshes(const tg3_model& model, /*std::vector<uint32_t>& material_ids,*/ SceneResources& scene_resources) {
    std::vector<uint32_t> mesh_ids(model.meshes_count);
    uint32_t num_vertices = 0;
    uint32_t num_indices = 0;
    for (uint32_t m = 0; m < model.meshes_count; m++) {
        const tg3_mesh* tg3_mesh = &model.meshes[m];
        for (uint32_t p = 0; p < tg3_mesh->primitives_count; p++) {
            const tg3_primitive* primitive = &tg3_mesh->primitives[p];

            if (primitive->indices != -1) {
                const tg3_accessor* accessor = &model.accessors[primitive->indices];
                num_indices += accessor->count;
            }

            if (primitive->attributes_count > 0) {
                const tg3_str_int_pair* attrib = &primitive->attributes[0];
                const tg3_accessor* accessor = &model.accessors[attrib->value];
                num_vertices += accessor->count;
            }
        }
    }

    std::vector<Vertex>& vertices = scene_resources.getVertices();
    std::vector<uint32_t>& indices = scene_resources.getIndices();

    vertices.reserve(num_vertices);
    indices.reserve(num_indices);

    for (uint32_t m = 0; m < model.meshes_count; m++) {
        const tg3_mesh* tg3_mesh = &model.meshes[m];
        Mesh mesh;
        mesh.primitives.resize(tg3_mesh->primitives_count);

        for (uint32_t p = 0; p < tg3_mesh->primitives_count; p++) {
            const tg3_primitive* primitive = &tg3_mesh->primitives[p];
            mesh.primitives[p].vertex_start = static_cast<uint32_t>(vertices.size());

            if (primitive->indices != -1) {
                const tg3_accessor* accessor = &model.accessors[primitive->indices];
                const tg3_buffer_view* buffer_view = &model.buffer_views[accessor->buffer_view];
                const tg3_buffer* buffer = &model.buffers[buffer_view->buffer];

                mesh.primitives[p].first_index = static_cast<uint32_t>(indices.size());
                mesh.primitives[p].index_count = accessor->count;

                if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* data = reinterpret_cast<const uint32_t*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                    indices.insert(indices.end(), data, data + accessor->count);
                } 
                else if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* data = reinterpret_cast<const uint16_t*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                    for (uint64_t i = 0; i < accessor->count; i++) {
                        indices.push_back(static_cast<uint32_t>(data[i]));
                    }
                }
            }

            struct Attribute { size_t offset; size_t stride; const std::byte* start; int num_floats; };
            std::vector<Attribute> attribs(primitive->attributes_count);
            for (uint32_t a = 0; a < primitive->attributes_count; a++) {
                const tg3_str_int_pair* attrib = &primitive->attributes[a];
                const tg3_accessor* accessor = &model.accessors[attrib->value];
                mesh.primitives[p].vertex_count = accessor->count;

                size_t field_offset = 0;
                int float_count = 0;
                if (strcmp(attrib->key.data, "POSITION") == 0) {
                    field_offset = offsetof(Vertex, position);
                    float_count = 3;
                } 
                else if (strcmp(attrib->key.data, "NORMAL") == 0) {
                    field_offset = offsetof(Vertex, normal);
                    float_count = 3;
                } 
                else if (strcmp(attrib->key.data, "TEXCOORD_0") == 0) {
                    field_offset = offsetof(Vertex, uv);
                    float_count = 2;
                } 
                else if(strcmp(attrib->key.data, "COLOR_0") == 0) {
                    assert(accessor->type == TG3_TYPE_VEC3);
                    field_offset = offsetof(Vertex, color);
                    float_count = 3;
                } 
                else continue;

                const tg3_buffer_view* buffer_view = &model.buffer_views[accessor->buffer_view];
                const tg3_buffer* buffer = &model.buffers[buffer_view->buffer];

                attribs[a].offset = field_offset; 
                attribs[a].stride = buffer_view->byte_stride != 0 ? buffer_view->byte_stride : sizeof(float) * float_count;
                attribs[a].start = reinterpret_cast<const std::byte*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                attribs[a].num_floats = float_count;
            }

            for (int v = 0; v < mesh.primitives[p].vertex_count; v++) {
                Vertex vertex;
                for (const Attribute& attrib : attribs) {
                    const void* src = reinterpret_cast<const void*>(attrib.start + (v * attrib.stride));
                    void* dst = reinterpret_cast<std::byte*>(&vertex) + attrib.offset;
                    memcpy(dst, src, attrib.num_floats * sizeof(float));
                }
                vertices.push_back(vertex);
            }
        }
        std::vector<Mesh>& meshes = scene_resources.getMeshes();
        uint32_t mesh_id = meshes.size();
        meshes.push_back(std::move(mesh));
        mesh_ids[m] = mesh_id;
    } 

    Buffer vertex_staging = createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(vertices[0]) * vertices.size(), true, VMA_MEMORY_USAGE_AUTO);
    Buffer index_staging = createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(indices[0]) * indices.size(), true, VMA_MEMORY_USAGE_AUTO);
    vertex_staging.map();
    index_staging.map();
    memcpy(vertex_staging.mapped, vertices.data(), sizeof(vertices[0]) * vertices.size());
    memcpy(index_staging.mapped, indices.data(), sizeof(indices[0]) * indices.size());
    vertex_staging.unmap();
    index_staging.unmap();

    Buffer vertex_buffer = createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, sizeof(vertices[0]) * vertices.size(), false, VMA_MEMORY_USAGE_AUTO);
    Buffer index_buffer = createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(indices[0]) * indices.size(), false, VMA_MEMORY_USAGE_AUTO);
    uint32_t vert_id = scene_resources.addBuffer(std::move(vertex_buffer));
    uint32_t index_id = scene_resources.addBuffer(std::move(index_buffer));
    scene_resources.setVertexBufferId(vert_id);
    scene_resources.setIndexBufferId(index_id);

    copyBuffer(vertex_staging, *scene_resources.getBuffer(vert_id), sizeof(vertices[0]) * vertices.size());
    copyBuffer(index_staging, *scene_resources.getBuffer(index_id), sizeof(indices[0]) * indices.size());

    return mesh_ids;
}

uint32_t GltfLoader::importNode(Tree& tree, const tg3_model& model, int32_t node_index, uint32_t parent_id, uint32_t prev_sibling_id, std::vector<uint32_t>& mesh_ids) {
    const tg3_node& tg3node = model.nodes[node_index];
    auto [node, node_id] = tree.createNode();
    node.m_parent_id = parent_id;

    if (tg3node.has_matrix) {
        glm::mat4 matrix(1.0f);
        float* matrix_ptr = glm::value_ptr(matrix);
        for (int i = 0; i < matrix.length(); i++) {
            matrix_ptr[i] = static_cast<float>(tg3node.matrix[i]);
        }
        node.m_matrix = matrix;
    }
    else {
        node.m_translation = glm::vec3(tg3node.translation[0], tg3node.translation[1], tg3node.translation[2]);
        node.m_rotation = glm::quat(tg3node.rotation[0], tg3node.rotation[1], tg3node.rotation[2], tg3node.rotation[3]);
        node.m_scale = glm::vec3(tg3node.scale[0], tg3node.scale[1], tg3node.scale[2]);
    }

    if (tg3node.mesh != -1) {
        node.m_mesh_id = mesh_ids[tg3node.mesh];
    }

    if (prev_sibling_id != UINT32_MAX) {
        tree.getNode(prev_sibling_id)->m_next_sibling_id = node_id;
    }

    uint32_t last_child_id = 0;
    for (int i = 0; i < tg3node.children_count; i++) {
        int32_t child_index = tg3node.children[i];
        last_child_id = importNode(tree, model, child_index, node_id, last_child_id, mesh_ids);

        if (node.m_first_child_id == UINT32_MAX) {
            node.m_first_child_id = last_child_id;
        }
    }

    return node_id;
}
