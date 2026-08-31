#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct SceneObject {
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	int modelIndex = -1;
};
