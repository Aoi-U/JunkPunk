#pragma once

#include <glm/glm.hpp>

struct Transform
{
	glm::vec3 position;
	glm::quat quatRotation;
	glm::vec3 scale;
};