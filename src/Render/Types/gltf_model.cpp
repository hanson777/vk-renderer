#include "gltf_model.h"
#include "Render/Types/Vertex.h"
#include <tiny_gltf_v3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

void Model::loadGltf(const std::string& filename) {
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

    m_buffers.resize(2);

    loadMeshes(model);

    tg3_model_free(&model);
    tg3_error_stack_free(&errors);
}

void Model::loadNode(const tg3_model& model) {
    
}

void Model::loadMeshes(const tg3_model& model) {
    // std::vector<uint32_t> mesh_ids(model.meshes_count);
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

    m_vertices.reserve(num_vertices);
    m_indices.reserve(num_indices);

    for (uint32_t m = 0; m < model.meshes_count; m++) {
        const tg3_mesh* tg3_mesh = &model.meshes[m];
        Mesh mesh;
        mesh.primitives.resize(tg3_mesh->primitives_count);

        for (uint32_t p = 0; p < tg3_mesh->primitives_count; p++) {
            const tg3_primitive* primitive = &tg3_mesh->primitives[p];
            mesh.primitives[p].vertex_start = static_cast<uint32_t>(m_vertices.size());

            if (primitive->indices != -1) {
                const tg3_accessor* accessor = &model.accessors[primitive->indices];
				const tg3_buffer_view* buffer_view = &model.buffer_views[accessor->buffer_view];
				const tg3_buffer* buffer = &model.buffers[buffer_view->buffer];

                mesh.primitives[p].first_index = static_cast<uint32_t>(m_indices.size());
                mesh.primitives[p].index_count = accessor->count;

                if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* data = reinterpret_cast<const uint32_t*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                    m_indices.insert(m_indices.end(), data, data + accessor->count);
                } 
                else if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* data = reinterpret_cast<const uint16_t*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                    for (uint64_t i = 0; i < accessor->count; i++) {
                        m_indices.push_back(static_cast<uint32_t>(data[i]));
                    }
                }
            }

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
                Vertex vertex{};
                for (const Attribute& attrib : attribs) {
                    const void* src = reinterpret_cast<const void*>(attrib.start + (v * attrib.stride));
                    void* dst = reinterpret_cast<std::byte*>(&vertex) + attrib.offset;
                    memcpy(dst, src, attrib.num_floats * sizeof(float));
                }
                m_vertices.push_back(vertex);
            }
        }
        m_meshes.push_back(std::move(mesh));
    } 
}
