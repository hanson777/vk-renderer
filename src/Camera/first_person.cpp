#include "first_person.h"
#include "Core/Input.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

FpsCamera::FpsCamera(glm::vec3 pos, glm::vec3 up)
    : m_pos(pos), m_up(up), m_yaw(-90.0f), m_pitch(0.0f),
      m_speed(10.0f), m_sensitivity(0.1f), m_zoom(45.0f) {
    updateCameraVectors();
}

void FpsCamera::update(float dt) {
    if (Input::KeyPressed(GLFW_KEY_W)) processKeyboard(Direction::Forward, dt);
    if (Input::KeyPressed(GLFW_KEY_S)) processKeyboard(Direction::Backward, dt);
    if (Input::KeyPressed(GLFW_KEY_A)) processKeyboard(Direction::Left, dt);
    if (Input::KeyPressed(GLFW_KEY_D)) processKeyboard(Direction::Right, dt);
    if (Input::KeyPressed(GLFW_KEY_SPACE)) processKeyboard(Direction::Up, dt);
    if (Input::KeyPressed(GLFW_KEY_LEFT_SHIFT)) processKeyboard(Direction::Down, dt);

    processMouseMovement(Input::g_mouse_delta.x, Input::g_mouse_delta.y);
    processMouseScroll(Input::g_scroll_delta);
}

void FpsCamera::processKeyboard(Direction d, float dt) {
    float velocity = m_speed * dt;
    switch (d) {
        case Direction::Forward: m_pos += m_flatFront * velocity; break;
        case Direction::Backward: m_pos -= m_flatFront * velocity; break;
        case Direction::Left: m_pos -= m_right * velocity; break;
        case Direction::Right: m_pos += m_right * velocity; break;
        case Direction::Up: m_pos.y += velocity; break;
        case Direction::Down: m_pos.y -= velocity; break;
    }
}

void FpsCamera::processMouseMovement(float xoffset, float yoffset) {
    m_yaw += xoffset * m_sensitivity;
    m_pitch += yoffset * m_sensitivity;
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
    updateCameraVectors();
}

void FpsCamera::processMouseScroll(float yoffset) {
    m_zoom = std::clamp(m_zoom - yoffset, 1.0f, 45.0f);
}

glm::mat4 FpsCamera::getViewMatrix() const {
    return glm::lookAtRH(m_pos, m_pos + m_front, m_up);
}

void FpsCamera::updateCameraVectors() {
    float yaw = glm::radians(m_yaw);
    float pitch = glm::radians(m_pitch);
    m_front = glm::normalize(glm::vec3(
        std::cos(yaw) * std::cos(pitch),
        std::sin(pitch),
        std::sin(yaw) * std::cos(pitch)
    ));
    m_flatFront = glm::normalize(glm::vec3(m_front.x, 0.0f, m_front.z));
    m_right = glm::normalize(glm::cross(m_front, m_up));
}
