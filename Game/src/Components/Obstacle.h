#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "../Core/Types.h"

struct MovingObstacle
{
	std::vector<glm::vec3> pathPoints; 
	std::vector<glm::quat> pathRotations; // can be empty. if provided, must be the same size as pathPoints
	float progress;
	int currentPathIndex;
	float speed; // units per second
	bool paused;
};