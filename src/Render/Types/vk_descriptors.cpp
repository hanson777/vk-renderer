#include "vk_descriptors.h"
#include "Render/vk_common.h"
#include "Render/Managers/vk_device.h"

VkDescriptorPool createDescriptorPool(VkDescriptorPoolSize pool_size) {
    VkDescriptorPoolCreateInfo pool_ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
    };

    VkDescriptorPool pool = VK_NULL_HANDLE;
    VK_CHECK(vkCreateDescriptorPool(vk_device::GetDevice(), &pool_ci, nullptr, &pool));

    return pool;
}

VkDescriptorSetLayout createTextureSetLayout(uint32_t capacity) {
    VkDescriptorSetLayoutBinding binding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = capacity,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutCreateInfo layout_ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &binding,
    };

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VK_CHECK(vkCreateDescriptorSetLayout(vk_device::GetDevice(), &layout_ci, nullptr, &layout));

    return layout;
}

VkDescriptorSet allocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };

    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
    VK_CHECK(vkAllocateDescriptorSets(vk_device::GetDevice(), &alloc_info, &descriptor_set));

    return descriptor_set;
}

void writeTextureDescriptors(VkDescriptorSet set, const std::vector<VkDescriptorImageInfo>& image_infos) {
    if (image_infos.empty()) return;

    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = static_cast<uint32_t>(image_infos.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = image_infos.data(),
    };

    vkUpdateDescriptorSets(vk_device::GetDevice(), 1, &write, 0, nullptr);
}
