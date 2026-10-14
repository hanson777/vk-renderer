#include "scene_resources.h"
#include "Render/Managers/vk_device.h"
#include "Render/Managers/vk_memory.h"
#include <tiny_gltf_v3.h>
#include <stb_image.h>
#include <climits>
#include <vector>
#include <iostream>

uint8_t* decodeEmbeddedImage(const tg3_model& model, const tg3_image& image, int& width, int& height) {
    if (image.buffer_view < 0 || static_cast<uint32_t>(image.buffer_view) >= model.buffer_views_count) {
        return nullptr;
    }

    const tg3_buffer_view& buffer_view = model.buffer_views[image.buffer_view];
    if (buffer_view.buffer < 0 || static_cast<uint32_t>(buffer_view.buffer) >= model.buffers_count) {
        return nullptr;
    }

    const tg3_span_u8& buffer_data = model.buffers[buffer_view.buffer].data;
    if (buffer_data.data == nullptr || buffer_view.byte_offset > buffer_data.count ||
        buffer_view.byte_length > buffer_data.count - buffer_view.byte_offset ||
        buffer_view.byte_length == 0 || buffer_view.byte_length > INT_MAX) {
        return nullptr;
    }

    int channels;
    return stbi_load_from_memory(buffer_data.data + buffer_view.byte_offset,
                                static_cast<int>(buffer_view.byte_length),
                                &width, &height, &channels, STBI_rgb_alpha);
}

void SceneResources::shutdown() {
    destroyDescriptors();
    destroyBuffers();
    destroyImages();
    destroySamplers();
}

bool SceneResources::prepareDescriptors(VkDescriptorSetLayout layout, uint32_t image_capacity, uint32_t sampler_capacity) {
    // Populate once before rendering; never overwrite a set that may be in flight.
    if (m_scene_descriptors.pool != VK_NULL_HANDLE) {
        std::cerr << "[ERROR::prepareDescriptors] scene descriptors already prepared\n";
        return false;
    }
    // Image slots follow texture IDs, including textures that share the same image.
    if (!checkDescriptorCounts(m_textures.size(), m_samplers.size(), image_capacity, sampler_capacity)) return false;
    if (m_fallback_texture_id >= m_textures.size()) {
        std::cerr << "[ERROR::prepareDescriptors] missing fallback texture\n";
        return false;
    }
    for (const Texture& texture : m_textures) {
        if (texture.image_id >= m_images.size() || texture.sampler_id >= m_samplers.size()) {
            std::cerr << "[ERROR::prepareDescriptors] texture references an invalid image or sampler\n";
            return false;
        }
    }
    for (const Material& material : m_materials) {
        if (material.texture_index >= m_textures.size() || material.sampler_index >= m_samplers.size()) {
            std::cerr << "[ERROR::prepareDescriptors] material references an invalid texture or sampler\n";
            return false;
        }
    }

    std::vector<VkDescriptorImageInfo> image_infos;
    std::vector<VkDescriptorImageInfo> sampler_infos;

    const Texture& fallback_texture = m_textures[m_fallback_texture_id];
    const VkDescriptorImageInfo fallback_info{
        .imageView = m_images[fallback_texture.image_id].image_view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    image_infos.resize(image_capacity, fallback_info);

    const VkDescriptorImageInfo fallback_sampler_info{
        .sampler = m_samplers[fallback_texture.sampler_id],
    };
    sampler_infos.resize(sampler_capacity, fallback_sampler_info);

    for (size_t i = 0; i < m_textures.size(); i++) {
        image_infos[i] = VkDescriptorImageInfo{
            .imageView = m_images[m_textures[i].image_id].image_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

    for (size_t i = 0; i < m_samplers.size(); i++) {
        sampler_infos[i] = VkDescriptorImageInfo{
            .sampler = m_samplers[i],
        };
    }

    m_scene_descriptors.pool = createDescriptorPool(image_capacity, sampler_capacity);
    m_scene_descriptors.textures = allocateDescriptorSet(m_scene_descriptors.pool, layout);
    writeTextureDescriptors(m_scene_descriptors.textures, image_infos);
    writeSamplerDescriptors(m_scene_descriptors.textures, sampler_infos);
    std::cout << "[INFO::SCENE_DESCRIPTORS] populated " << m_textures.size() << '/' << image_capacity
              << " images, " << m_samplers.size() << '/' << sampler_capacity << " samplers; unused slots use fallbacks\n";
    return true;
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
