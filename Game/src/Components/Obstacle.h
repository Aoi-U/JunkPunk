#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "../Core/Types.h"

struct MovingObstacle
{
	std::vector<glm::vec3> pathPoints; 
	float progress;
	int currentPathIndex;
	float speed; // units per second
};