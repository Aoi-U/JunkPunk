#pragma once

#include <glm/glm.hpp>
#include "../Core/Types.h"

struct DangerZone
{
	glm::vec3 center = glm::vec3(0.0f);
	glm::vec3 halfExtents = glm::vec3(0.0f);
	Entity obstacleEntity = 0;  // the MovingObstacle this zone belongs to
	float padding = 5.0f;       // extra width around the glove's path
};