#include "gltf_model.h"
#include "Types/Vertex.h"
#include <tiny_gltf_v3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

void Model::loadGltf(std::string& filename) {
    tg3_parse_options opts;
    tg3_error_stack errors;
    tg3_model model;

    tg3_parse_options_init(&opts);
    tg3_error_stack_init(&errors);

    tg3_error_code err = tg3_parse_file(&model, &errors, filename.c_str(), 10, &opts);
    if (err != TG3_OK) {
        for (uint32_t i = 0; i < errors.count; i++) {
            std::cerr << "[ERROR::loadModels] " << "Severity: " << errors.entries[i].severity <<
                    (errors.entries[i].message ? errors.entries[i].message : "(null)");
        }
    }

    loadMeshes(model);

    tg3_model_free(&model);
    tg3_error_stack_free(&errors);
}

void Model::loadMeshes(const tg3_model& model) {
    std::vector<uint32_t> mesh_ids(model.meshes_count);
    for (int i = 0; i < model.meshes_count; i++) {
        const tg3_mesh* tg3_mesh = &model.meshes[i];
        Mesh mesh;
        mesh.primitives.resize(tg3_mesh->primitives_count);
        for (int i = 0; i < tg3_mesh->primitives_count; i++) {
            const tg3_primitive* primitive = &tg3_mesh->primitives[i];
            mesh.primitives[i].vertex_start = m_vert_offset;

            if (primitive->indices != -1) {
                const tg3_accessor* accessor = &model.accessors[primitive->indices];
				const tg3_buffer_view* buffer_view = &model.buffer_views[accessor->buffer_view];
				const tg3_buffer* buffer = &model.buffers[buffer_view->buffer];

                mesh.primitives[i].first_index = m_index_offset;
                mesh.primitives[i].index_count = accessor->count;

                if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* data = reinterpret_cast<const uint32_t*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                    memcpy(&m_indices[m_index_offset], data, accessor->count * sizeof(uint32_t));
                } 
                else if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* data = reinterpret_cast<const uint16_t*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                    for (uint64_t i = 0; i < accessor->count; i++) {
                        m_indices[i + m_index_offset] = static_cast<uint32_t>(data[i]);
                    }
                }
                m_index_offset += accessor->count;
            }

            std::vector<Attribute> attribs(primitive->attributes_count);
            for (int a = 0; a < primitive->attributes_count; a++) {
                const tg3_str_int_pair* attrib = &primitive->attributes[a];
                const tg3_accessor* accessor = &model.accessors[attrib->value];
                mesh.primitives[i].vertex_count = accessor->count;

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
                else if (strcmp(attrib->key.data, "TEX_COORD0") == 0) {
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
                const tg3_buffer* buffer = &model.buffers[accessor->buffer_view];

                attribs[i].offset = field_offset; 
                attribs[i].stride = buffer_view->byte_stride != 0 ? buffer_view->byte_stride : sizeof(float) * attribs[i].num_floats;
                attribs[i].start = reinterpret_cast<const std::byte*>(buffer->data.data + buffer_view->byte_offset + accessor->byte_offset);
                attribs[i].num_floats = float_count;
            }

            for (int v = 0; v < mesh.primitives[i].vertex_count; v++) {
                for (const Attribute& attrib : attribs) {
                    const void* src = reinterpret_cast<const void*>(attrib.start + (v * attrib.stride));
                    void* dst = reinterpret_cast<std::byte*>(&m_vertices[v]) + attrib.offset;
                    memcpy(dst, src, attrib.num_floats * sizeof(float));
                }
            }
            m_vert_offset += mesh.primitives[i].vertex_count;
        }
    } 
}
