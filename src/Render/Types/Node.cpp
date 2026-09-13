#include "Node.h"
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

void Node::setMatrix(glm::mat4& matrix) {
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(matrix, m_scale, m_rotation, m_translation, skew, perspective);
    m_matrix = matrix;
    m_dirty = false;
}
