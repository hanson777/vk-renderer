#pragma once
#include "Render/vk_common.h"
#include <vector>
#include <cstddef>

struct SceneDescriptors {
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSet textures = VK_NULL_HANDLE;
};

VkDescriptorPool createDescriptorPool(uint32_t image_capacity, uint32_t sampler_capacity);
VkDescriptorSetLayout createTextureSetLayout(uint32_t image_capacity, uint32_t sampler_capacity);
VkDescriptorSet allocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout);
bool checkDescriptorCounts(size_t image_count, size_t sampler_count, uint32_t image_capacity, uint32_t sampler_capacity);
void writeTextureDescriptors(VkDescriptorSet set, const std::vector<VkDescriptorImageInfo>& image_infos);
void writeSamplerDescriptors(VkDescriptorSet set, const std::vector<VkDescriptorImageInfo>& sampler_infos);
