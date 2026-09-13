#pragma once
#include "Render/Types/scene_resources.h"
#include "Render/Types/Tree.h"
#include <tiny_gltf_v3.h>
#include <vector>
#include <cstdint>

struct GltfLoader {
    void loadGltf(const std::string& filename, SceneResources& scene_resources);
    
    private:
    std::vector<uint32_t> loadSamplers(const tg3_model& model, SceneResources& scene_resources);
    std::vector<uint32_t> loadTextures(const tg3_model& model, std::vector<uint32_t>& image_ids, SceneResources& scene_resources);
    std::vector<uint32_t> loadMaterials(const tg3_model& model, std::vector<uint32_t>& texture_ids, SceneResources& scene_resources);
    std::vector<uint32_t> loadMeshes(const tg3_model& model, /*std::vector<uint32_t>& material_ids,*/SceneResources& scene_resources);

    uint32_t importNode(Tree& tree, const tg3_model& model, int32_t node_index, uint32_t parent_id, uint32_t prev_sibling_id, std::vector<uint32_t>& mesh_ids);
};
