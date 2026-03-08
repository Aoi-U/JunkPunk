#pragma once

#include "System.h"
#include <glm/glm.hpp>
#include <vector>

struct Waypoint {
	glm::vec3 position;
	float recommendedSpeed = 10.0f;
	float trackWidth = 5.0f;
};

class AiSystem : public System
{
public:
	void Init();
	void Update(float deltaTime);
	void RenderDebugWaypoints(); // New method for debug visualization

	const std::vector<Waypoint>& GetWaypoints() const { return trackWaypoints; }

private:
	std::vector<Waypoint> trackWaypoints;

	void InitializeWaypoints();
};