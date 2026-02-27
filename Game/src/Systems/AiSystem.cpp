#include "AiSystem.h"
#include "../Components/Render.h"
#include "../Components/Transform.h"
#include "../Components/Physics.h"
#include "../Components/Player.h"
#include "../Components/AiDriver.h"
#include "../ECSController.h"
#include <cmath>

extern ECSController controller;

void AiSystem::Init()
{
	InitializeWaypoints();
}

void AiSystem::Update(float deltaTime)
{
	//std::cout << "Updating AiSystem with " << entities.size() << " entities" << std::endl;
	if (deltaTime <= 0.0f)
		return;

	// For each entity matched by system signature
	for (auto entity : entities)
	{
		if (!controller.HasComponent<AiDriver>(entity) ||
			!controller.HasComponent<Transform>(entity) ||
			!controller.HasComponent<VehicleBody>(entity) ||
			!controller.HasComponent<VehicleCommands>(entity))
		{
			continue;
		}

		auto& ai = controller.GetComponent<AiDriver>(entity);
		auto& transform = controller.GetComponent<Transform>(entity);
		auto& body = controller.GetComponent<VehicleBody>(entity);
		auto& vc = controller.GetComponent<VehicleCommands>(entity);

		// clamp index
		if (ai.currentWaypointIndex >= trackWaypoints.size())
			ai.currentWaypointIndex = 0;

		glm::vec3 posXZ(transform.position.x, 0.0f, transform.position.z);
		glm::vec3 wpPos = trackWaypoints[ai.currentWaypointIndex].position;
		glm::vec3 wpXZ(wpPos.x, 0.0f, wpPos.z);

		// distance to current waypoint
		float dist = glm::length(wpXZ - posXZ);

		// advance when within arrival radius
		// Use A* to find path between waypoints and follow that path instead of straight lines, this will allow the AI to navigate around obstacles and take better racing lines
		if (dist < ai.arrivalRadius)
		{
			ai.currentWaypointIndex = (ai.currentWaypointIndex + 1) % trackWaypoints.size();
			// update target waypoint values
			wpPos = trackWaypoints[ai.currentWaypointIndex].position;
			wpXZ = glm::vec3(wpPos.x, 0.0f, wpPos.z);
			dist = glm::length(wpXZ - posXZ);
		}

		// target direction (XZ)
		glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
		forward.y = 0.0f;
		if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
		forward = glm::normalize(forward);

		glm::vec3 toTarget = wpXZ - posXZ;
		glm::vec3 toTargetN = glm::length(toTarget) > 1e-5f ? glm::normalize(toTarget) : forward;

		// signed angle in XZ plane
		// Add print statements to check for turning
		// Atan2 function to help with turning/angles
		// replace your signed angle with atan2(cross, dot) <-- standard gamedegv way
		auto signedAngleXZ = [](const glm::vec3& a, const glm::vec3& b) -> float {
			float dot = glm::clamp(glm::dot(glm::normalize(a), glm::normalize(b)), -1.0f, 1.0f);
			float angle = std::acos(dot);
			float crossY = a.x * b.z - a.z * b.x;
			return crossY < 0.0f ? -angle : angle;
			};

		float headingError = signedAngleXZ(forward, toTargetN);

		// Simple steering: proportional to heading error, normalized to [-1,1]
		float steer = glm::clamp(headingError / ai.maxSteerRadians, -1.0f, 1.0f);

		// Simple throttle:
		// - drive toward recommended speed (or ai.desiredSpeed)
		float targetSpeed = (trackWaypoints[ai.currentWaypointIndex].recommendedSpeed > 0.0f)
			? trackWaypoints[ai.currentWaypointIndex].recommendedSpeed
			: ai.desiredSpeed;
		float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));

		// Basic proportional throttle; reduce throttle for large heading errors
		float baseThrottle = glm::clamp((targetSpeed - speed) * ai.throttleKp, 0.0f, ai.maxThrottle);
		if (std::abs(headingError) > ai.brakeAngleThreshold)
			baseThrottle *= 0.3f; // slow down into corners

		// Simple brake when very close and still too fast
		float brake = 0.0f;
		if (dist < ai.arrivalRadius * 1.2f && speed > targetSpeed + 1.0f)
			brake = glm::clamp((speed - targetSpeed) * 0.5f, 0.0f, ai.maxBrake);

		// Write commands consumed by PhysicsSystem/MainVehicle
		vc.steer = steer;
		vc.throttle = baseThrottle;
		vc.brake = brake;
		vc.isGrounded = true;
	}
}

void AiSystem::InitializeWaypoints()
{
	// Create waypoints in a circle
	// Center of the circle near player spawn
	// Just using this for testing
	glm::vec3 circleCenter(-80.0f, -93.0f, 19.0f);
	float circleRadius = 5.0f;
	int numWaypoints = 8; // Start with 8 waypoints around the circle

	trackWaypoints.clear();
	trackWaypoints.reserve(numWaypoints);

	for (int i = 0; i < numWaypoints; i++)
	{
		float angle = (2.0f * glm::pi<float>() * i) / numWaypoints;

		Waypoint wp;
		wp.position.x = circleCenter.x + circleRadius * std::cos(angle);
		wp.position.y = circleCenter.y; // Keep at same Y level
		wp.position.z = circleCenter.z + circleRadius * std::sin(angle); // Improve on this
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