#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>

struct Node {
    public:
    const glm::vec3& getTranslation() const { return m_translation; }
    const glm::quat& getRotation() const { return m_rotation; }
    const glm::vec3& getScale() const { return m_scale; }
    const glm::mat4& getMatrix() const;

    void setTranslation(const glm::vec3& translation) { 
        m_translation = translation; 
        m_dirty = true; 
    }

    void setRotation(const glm::quat& rotation) { 
        m_rotation = rotation; 
        m_dirty = true; 
    }

    void setScale(const glm::vec3& scale) { 
        m_scale = scale; 
        m_dirty = true; 
    }

    void setMeshId(uint32_t id) { 
        m_mesh_id = id; 
    }

    void setMatrix(const glm::mat4& matrix);  

    uint32_t getMeshId() const { return m_mesh_id; }
    uint32_t getParentId() const { return m_parent_id; }
    uint32_t getNextSiblingId() const { return m_next_sibling_id; }
    uint32_t getFirstChildId() const { return m_first_child_id; }

    private:
    friend struct Tree;

	glm::vec3 m_translation = glm::vec3(0.0f);
    glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f);
	mutable glm::mat4 m_matrix = glm::mat4(1.0f);
    mutable bool m_dirty = true;

    uint32_t m_mesh_id = UINT32_MAX; 
    uint32_t m_parent_id = UINT32_MAX; 
    uint32_t m_next_sibling_id = UINT32_MAX;
    uint32_t m_first_child_id = UINT32_MAX;
};
