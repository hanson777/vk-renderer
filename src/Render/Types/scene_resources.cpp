#include "scene_resources.h"
#include <vector>
#include <iostream>

uint32_t SceneResources::addBuffer(Buffer buffer) {
    uint32_t id = m_buffers.size();
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
