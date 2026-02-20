#include "AiSystem.h"
#include "../Components/Render.h"
#include "../Components/Transform.h"
#include "../ECSController.h"
#include <cmath>

extern ECSController controller;

void AiSystem::Init()
{
	InitializeWaypoints();
}

void AiSystem::Update(float deltaTime)
{
	// TODO: AI logic will go here in later steps
}

void AiSystem::InitializeWaypoints()
{
	// Create waypoints in a circle
	// Center of the circle near player spawn
	glm::vec3 circleCenter(0.0f, -5.0f, 0.0f);
	float circleRadius = 20.0f;
	int numWaypoints = 8; // Start with 8 waypoints around the circle

	trackWaypoints.clear();
	trackWaypoints.reserve(numWaypoints);

	for (int i = 0; i < numWaypoints; i++)
	{
		float angle = (2.0f * glm::pi<float>() * i) / numWaypoints;

		Waypoint wp;
		wp.position.x = circleCenter.x + circleRadius * std::cos(angle);
		wp.position.y = circleCenter.y; // Keep at same Y level
		wp.position.z = circleCenter.z + circleRadius * std::sin(angle);
		wp.recommendedSpeed = 15.0f;
		wp.trackWidth = 5.0f;

		trackWaypoints.push_back(wp);
	}
}

void AiSystem::RenderDebugWaypoints()
{
	// This will draw waypoint positions to console
	static bool printed = false;
	if (!printed)
	{
		std::cout << "=== Waypoints ===" << std::endl;
		for (size_t i = 0; i < trackWaypoints.size(); i++)
		{
			const auto& wp = trackWaypoints[i];
			std::cout << "Waypoint " << i << ": ("
				<< wp.position.x << ", "
				<< wp.position.y << ", "
				<< wp.position.z << ")" << std::endl;
		}
		printed = true;
	}
}