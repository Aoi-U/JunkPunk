#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "../Core/Types.h"

struct MovingObstacle
{
	std::vector<glm::vec3> pathPoints; 
	std::vector<glm::quat> pathRotations; // new rotation at each path point. if empty, obstacle will not rotate
	std::vector<float> pathTimes; // time in seconds to reach each point. if empty, movement will be speed based
	float progress;
	int currentPathIndex;
	float speed; // units per second
	bool paused;
};