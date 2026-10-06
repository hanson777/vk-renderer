#pragma once
#include "Render/vk_common.h"
#include <vector>

struct SceneDescriptors {
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSet textures = VK_NULL_HANDLE;
};

VkDescriptorPool createDescriptorPool(VkDescriptorPoolSize pool_size);
VkDescriptorSetLayout createTextureSetLayout(uint32_t capacity);
VkDescriptorSet allocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout);
void writeTextureDescriptors(VkDescriptorSet set, const std::vector<VkDescriptorImageInfo>& image_infos);
