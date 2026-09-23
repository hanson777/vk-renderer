#include "scene_resources.h"
#include "Render/Managers/vk_device.h"
#include "Render/Managers/vk_memory.h"
#include <vector>
#include <iostream>

void SceneResources::shutdown() {
    destroyBuffers();
    destroyImages();
    destroySamplers();
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
