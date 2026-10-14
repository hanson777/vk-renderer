#include "vk_descriptors.h"
#include "Render/vk_common.h"
#include "Render/Managers/vk_device.h"
#include <array>
#include <iostream>

bool checkDescriptorCounts(size_t image_count, size_t sampler_count, uint32_t image_capacity, uint32_t sampler_capacity) {
    if (image_count > image_capacity || sampler_count > sampler_capacity) {
        std::cerr << "[ERROR::SCENE_DESCRIPTORS] scene requires " << image_count << " image descriptors and "
                  << sampler_count << " sampler descriptors (including fallbacks), but capacity is "
                  << image_capacity << " images and " << sampler_capacity << " samplers\n";
        return false;
    }
    return true;
}

VkDescriptorPool createDescriptorPool(uint32_t image_capacity, uint32_t sampler_capacity) {
    std::array<VkDescriptorPoolSize, 2> pool_sizes{
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = image_capacity,
        },
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = sampler_capacity,
        },
    };

    VkDescriptorPoolCreateInfo pool_ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 1,
        .poolSizeCount = pool_sizes.size(),
        .pPoolSizes = pool_sizes.data(),
    };

    VkDescriptorPool pool = VK_NULL_HANDLE;
    VK_CHECK(vkCreateDescriptorPool(vk_device::GetDevice(), &pool_ci, nullptr, &pool));
    return pool;
}

VkDescriptorSetLayout createTextureSetLayout(uint32_t image_capacity, uint32_t sampler_capacity) {
    std::array<VkDescriptorSetLayoutBinding, 2> bindings{
        VkDescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = image_capacity,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        },
        VkDescriptorSetLayoutBinding{
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = sampler_capacity,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        },
    };

    std::array<VkDescriptorBindingFlags, bindings.size()> binding_flags{
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
        0,
    };
    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = binding_flags.size(),
        .pBindingFlags = binding_flags.data(),
    };
    VkDescriptorSetLayoutCreateInfo layout_ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &binding_flags_ci,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = bindings.size(),
        .pBindings = bindings.data(),
    };

    VkDescriptorSetLayoutSupport support{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT,
    };
    vkGetDescriptorSetLayoutSupport(vk_device::GetDevice(), &layout_ci, &support);
    if (!support.supported) {
        std::cerr << "[ERROR::createTextureSetLayout] update-after-bind layout unsupported for "
                  << image_capacity << " images and " << sampler_capacity << " samplers\n";
        return VK_NULL_HANDLE;
    }

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
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = image_infos.data(),
    };

    vkUpdateDescriptorSets(vk_device::GetDevice(), 1, &write, 0, nullptr);
}

void writeSamplerDescriptors(VkDescriptorSet set, const std::vector<VkDescriptorImageInfo>& sampler_infos) {
    if (sampler_infos.empty()) return;

    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set,
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = static_cast<uint32_t>(sampler_infos.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = sampler_infos.data(),
    };

    vkUpdateDescriptorSets(vk_device::GetDevice(), 1, &write, 0, nullptr);
}
