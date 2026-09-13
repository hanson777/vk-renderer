#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>

struct Node {
	glm::vec3 m_translation = glm::vec3(0.0f);
    glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f);
	glm::mat4 m_matrix = glm::mat4(1.0f);
    bool m_dirty = true;

    uint32_t m_mesh_id = UINT32_MAX; 
    uint32_t m_parent_id = UINT32_MAX; 
    uint32_t m_next_sibling_id = UINT32_MAX;
    uint32_t m_first_child_id = UINT32_MAX;

    void setMatrix(glm::mat4& matrix);
};
