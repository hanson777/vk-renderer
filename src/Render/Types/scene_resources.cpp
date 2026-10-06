#include "scene_resources.h"
#include "Render/Managers/vk_device.h"
#include "Render/Managers/vk_memory.h"
#include <vector>
#include <iostream>

void SceneResources::shutdown() {
    destroyDescriptors();
    destroyBuffers();
    destroyImages();
    destroySamplers();
}

void SceneResources::prepareDescriptors(VkDescriptorSetLayout layout, uint32_t capacity) {
    std::vector<VkDescriptorImageInfo> image_infos;

    const VkDescriptorImageInfo fallback_info = image_infos[m_fallback_texture_id];
    image_infos.resize(capacity, fallback_info);

    for (int i = 0; i < m_textures.size(); i++) {
        image_infos[i] = VkDescriptorImageInfo{
            .sampler = m_samplers[m_textures[i].sampler_id],
            .imageView = m_images[m_textures[i].image_id].image_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

    m_scene_descriptors.pool = createDescriptorPool(VkDescriptorPoolSize{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = capacity,
    });
    m_scene_descriptors.textures = allocateDescriptorSet(m_scene_descriptors.pool, layout);
    writeTextureDescriptors(m_scene_descriptors.textures, image_infos);
}

void SceneResources::destroyDescriptors() {
    if (m_scene_descriptors.pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(vk_device::GetDevice(), m_scene_descriptors.pool, nullptr);
        m_scene_descriptors = {};
    }
}

uint32_t SceneResources::addBuffer(Buffer buffer) {
    uint32_t id = static_cast<uint32_t>(m_buffers.size());
    m_buffers.push_back(std::move(buffer));
    return id;
}

Buffer* SceneResources::getBuffer(uint32_t id) {
    if (id < 0 || id >= m_buffers.size()) {
        std::cerr << "[ERROR::SCENE_RESOURCES] getBuffer() index out of bounds\n";
        return nullptr;
    }
    return &m_buffers[id];
}

void SceneResources::destroyImages() {
    for (GpuImage& img : m_images) {
        vkDestroyImageView(vk_device::GetDevice(), img.image_view, nullptr);
        vmaDestroyImage(vk_memory::GetAllocator(), img.image, img.allocation);
    }
}

void SceneResources::destroySamplers() {
    for (VkSampler& sampler : m_samplers) {
        vkDestroySampler(vk_device::GetDevice(), sampler, nullptr);
    }
}
