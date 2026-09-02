#pragma once
#include "AssetManager.h"
#include "Types/Vertex.h"
#include "tiny_gltf_v3.h"
#include <vector>
#include <iostream>

namespace AssetManager {
    std::vector<Vertex> g_vertices;
    std::vector<uint32_t> g_indices;

    void LoadModels(const std::string& filename) {
        tg3_parse_options opts;
        tg3_error_stack errors;
        tg3_model model;

        tg3_parse_options_init(&opts);
        tg3_error_stack_init(&errors);

        tg3_error_code err = tg3_parse_file(&model, &errors, filename.c_str(), 10, &opts);
        if (err != TG3_OK) {
            for (uint32_t i = 0; i < errors.count; i++) {
                std::cerr << "[ERROR::LoadModels] " << "Severity: " << errors.entries[i].severity <<
                        (errors.entries[i].message ? errors.entries[i].message : "(null)");
            }
        }
        // ... use model ...
        tg3_model_free(&model);
        tg3_error_stack_free(&errors);
    }
}
