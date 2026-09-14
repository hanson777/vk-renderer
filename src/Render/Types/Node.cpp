#include "Node.h"
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

const glm::mat4& Node::getMatrix() const {
    if (m_dirty) {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), m_translation);
        glm::mat4 r = glm::mat4_cast(m_rotation);
        glm::mat4 s = glm::scale(glm::mat4(1.0f), m_scale);
        m_matrix = t * r * s;
        m_dirty = false;
    }
    return m_matrix;
}

void Node::setMatrix(const glm::mat4& transform) {
    glm::vec3 skew = glm::vec3(0.0f);
    glm::vec4 perspective = glm::vec4(0.0f);
    glm::decompose(transform, m_scale, m_rotation, m_translation, skew, perspective);
    m_matrix = transform;
    m_dirty = false;
}

