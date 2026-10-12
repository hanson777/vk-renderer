#pragma once
#include <glm/glm.hpp>

struct FpsCamera {
    private:
    enum class Direction { Forward, Backward, Left, Right, Up, Down };
    glm::vec3 m_pos;
    glm::vec3 m_up;
    glm::vec3 m_front;
    glm::vec3 m_flatFront;
    glm::vec3 m_right;
    
    float m_yaw;
    float m_pitch;
    float m_speed;
    float m_sensitivity;
    float m_zoom;

    void processKeyboard(Direction d, float dt);
    void processMouseMovement(float xoffset, float yoffset);
    void processMouseScroll(float yoffset);
    void updateCameraVectors();

    public:
    FpsCamera(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));

    void update(float dt);

    glm::mat4 getViewMatrix() const;

    glm::vec3 getPosition() const { return m_pos; }
    glm::vec3 getFront() const { return m_front; }
    glm::vec3 getFlatFront() const { return m_flatFront; }

    float getFov() const { return m_zoom; }
};
