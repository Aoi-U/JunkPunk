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

		if (trackWaypoints.empty())
			return; // nothing to drive to

		// Position of vehicle
		glm::vec3 positionXZ(transform.position.x, 0.0f, transform.position.z);
		glm::vec3 waypointPosition = trackWaypoints[ai.currentWaypointIndex].position;
		// Position of Waypoint
		glm::vec3 waypointXZ(waypointPosition.x, 0.0f, waypointPosition.z);

		//std::cout << "Next Waypoint Index: " << ai.currentWaypointIndex+1
		//	<< "\tNext Waypoint: (" << waypointXZ.x << "," << waypointXZ.y << "," << waypointXZ.z << ")"
		//	<< std::endl;

		// distance to current waypoint
		float dist = glm::length(waypointXZ - positionXZ);
		//std::cout << "Distance to waypoint " << ai.currentWaypointIndex << ": " << dist << std::endl;


		// target forward direction (XZ)
		glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
		forward.y = 0.0f;
		if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
		forward = glm::normalize(forward);

		// Previous waypoint
		int prevIndex = (ai.currentWaypointIndex - 1 + trackWaypoints.size()) % trackWaypoints.size();
		glm::vec3 prevPos = trackWaypoints[prevIndex].position;
		glm::vec3 prevXZ(prevPos.x, 0.0f, prevPos.z);


		// Normalized direction segment between next and previous waypoint
		glm::vec3 segmentDir = glm::normalize(waypointXZ - prevXZ);
		// Vehicle position relative to previous waypoint
		glm::vec3 toCar = positionXZ - prevXZ;
		// Project car onto segment
		float t = glm::dot(toCar, segmentDir);

		// advance when within arrival radius
		// Use A* to find path between waypoints and follow that path instead of straight lines, this will allow the AI to navigate around obstacles and take better racing lines
		float segmentLength = glm::length(waypointXZ - prevXZ);
		if (t + ai.lookaheadDistance > segmentLength)
		{
			ai.currentWaypointIndex = (ai.currentWaypointIndex + 1) % trackWaypoints.size();
		}

		// Add lookahead distance
		//float lookahead = ai.lookaheadDistance > 0.0f ? ai.lookaheadDistance : 5.0f;
		// Make this speed based instead of constant, so the AI can slow down for turns and speed up on straights
		float lookahead = ai.lookaheadDistance;
		glm::vec3 lookaheadPoint = prevXZ + segmentDir * (t + lookahead);

		//std::cout << "Lookahead Point: (" << lookaheadPoint.x << "," << lookaheadPoint.y << "," << lookaheadPoint.z << ")" << "\t\tCurrent location: " << positionXZ.x << "," << positionXZ.y << "," << positionXZ.z << std::endl;

		//std::cout << "Current Waypoint Index: " << ai.currentWaypointIndex
		//	<< "\tNext Waypoint: (" << waypointXZ.x << "," << waypointXZ.y << "," << waypointXZ.z << ")"
		//	<< std::endl;

		// Distance to target from lookaheadPoint
		glm::vec3 toTarget = lookaheadPoint - waypointXZ;
		glm::vec3 toTargetN = glm::length(toTarget) > 1e-5f ? glm::normalize(toTarget) : forward;

		float steer = 0.0f;
		// signed angle in XZ plane
		auto signedAngleXZ = [](const glm::vec3& a, const glm::vec3& b) -> float {
			glm::vec2 a2(a.x, a.z);
			glm::vec2 b2(b.x, b.z);
			float cross = a2.x * b2.y - a2.y * b2.x;
			float dot = glm::clamp(a2.x * b2.x + a2.y * b2.y, -1.0f, 1.0f);
			return std::atan2(cross, dot);
			};

		//float headingError = signedAngleXZ(forward, toTarget);

		//// Simple steering: proportional to heading error, normalized to [-1,1]
		//float steer = glm::clamp(headingError * 0.2f, -1.0f, 1.0f);

		float forwardDot = glm::clamp(glm::dot(forward, toTargetN), -1.0f, 1.0f);
		if (std::abs(forwardDot) > ai.steerDeadzoneDot)
		{
			steer = 0.0f;
		}
		else
		{
			float headingError = signedAngleXZ(forward, toTarget);
			// Simple steering: proportional to heading error, normalized to [-1,1]
			steer = glm::clamp(headingError * 0.2f, -1.0f, 1.0f);
		}

		// Simple throttle:
		// - drive toward recommended speed (or ai.desiredSpeed)
		float targetSpeed = (trackWaypoints[ai.currentWaypointIndex].recommendedSpeed > 0.0f)
			? trackWaypoints[ai.currentWaypointIndex].recommendedSpeed
			: ai.desiredSpeed;
		float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));

		//std::cout << "headingError: " << headingError
		//	<< "\t\tsteer: " << steer
		//	<< "\t\tspeed: " << speed
		//	<< std::endl;

		// Basic proportional throttle
		float baseThrottle = glm::clamp((targetSpeed - speed) * ai.throttleKp, 0.0f, ai.maxThrottle);

		float brake = 0.0f;

		// Write commands consumed by PhysicsSystem/MainVehicle
		vc.steer = steer;
		vc.throttle = baseThrottle;
		vc.brake = brake;
		vc.isGrounded = true;
	}
}

void AiSystem::InitializeWaypoints()
{
	trackWaypoints.clear();

	std::vector<glm::vec3> anchors{
		glm::vec3(-40.0f, -94.0f, -5.0f), // Waypoint near computer opponent
		glm::vec3(-80.0f, -93.0f, 19.0f),
		glm::vec3(-49.266, -84.591, 49.396),
		glm::vec3(11.345, -70.691, 12.822),
		glm::vec3(22, -70.194, 22),
		glm::vec3(-28.125, -59.594, 65.995),
		glm::vec3(7.254, -43.482, 95.625),
		glm::vec3(29.236, -38.011, 71.075),
		glm::vec3(21.665, -39.062, 56.706),
		glm::vec3(39.063, -38.777, 39.908),
		glm::vec3(81.944, -19.593, 81.963),
		glm::vec3(25.0f, -3.5f, 120.0f), // finish line
	};

	int nBetween = 3;

	// Build trackWaypoints: include first anchor, then for each segment add nBetween interior points.
	if (anchors.empty())
		return;

	for (size_t i = 0; i < anchors.size() - 1; ++i)
	{
		// push start anchor of this segment
		Waypoint startWp;
		startWp.position = anchors[i];
		startWp.recommendedSpeed = 12.0f;
		startWp.trackWidth = 5.0f;
		trackWaypoints.push_back(startWp);

		// generate interior points (exclude endpoints)
		for (int k = 1; k <= nBetween; ++k)
		{
			float t = static_cast<float>(k) / static_cast<float>(nBetween + 1); // evenly spaced in (0,1)
			glm::vec3 p = glm::mix(anchors[i], anchors[i + 1], t);

			Waypoint wp;
			wp.position = p;
			wp.recommendedSpeed = 12.0f;
			wp.trackWidth = 5.0f;
			trackWaypoints.push_back(wp);
		}
	}

	// push final anchor
	Waypoint lastWp;
	lastWp.position = anchors.back();
	lastWp.recommendedSpeed = 12.0f;
	lastWp.trackWidth = 5.0f;
	trackWaypoints.push_back(lastWp);
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