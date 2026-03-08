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
	//std::cout << "Initializing Waypoints" << std::endl;
	InitializeWaypoints();
}

void AiSystem::Update(float deltaTime)
{
	//std::cout << "Zoom Zoom" << std::endl;
	//if (deltaTime <= 0.0f)
	//	return;

	// For each entity matched by system signature
	for (auto entity : entities)
	{
		//if (!controller.HasComponent<AiDriver>(entity) ||
		//	!controller.HasComponent<Transform>(entity) ||
		//	!controller.HasComponent<VehicleBody>(entity) ||
		//	!controller.HasComponent<VehicleCommands>(entity))
		//{
		//	continue;
		//}

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

		//std::cout << "Current Waypoint Index: " << ai.currentWaypointIndex << "\tNext Waypoint Index: " << ai.currentWaypointIndex+1 << std::endl;

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

		//if (dist < ai.arrivalRadius || (t + ai.lookaheadDistance) > segmentLength)
		//{
		//	ai.currentWaypointIndex = (ai.currentWaypointIndex + 1) % trackWaypoints.size();
		//	// Skip steering calculation this frame so we start targeting the next waypoint next update.
		//	continue;
		//}
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
		glm::vec3(-39.0f, -94.0f, -8.0f), // Waypoint near computer opponent
		glm::vec3(-80.0f, -93.0f, 19.0f),
		glm::vec3(-49.266, -84.591, 49.396),
		glm::vec3(-2.345, -70.691, 12.822),
		glm::vec3(22, -70.194, 32),
		glm::vec3(-3.312, -61.604, 35.852),
		glm::vec3(-10.312, -61.604, 45.852),
		glm::vec3(-28.125, -59.594, 65.995),
		glm::vec3(4.254, -43.482, 101.625),
		glm::vec3(29.236, -38.011, 71.075),
		glm::vec3(21.665, -39.062, 56.706),
		glm::vec3(39.063, -38.777, 39.908),
		glm::vec3(81.944, -19.593, 81.963),
		glm::vec3(25.0f, -3.5f, 120.0f), // finish line
//		glm::vec3(-44.5055, -94.5461, -26.1023),
//glm::vec3(-44.3598, -94.5471, -22.1468),
//glm::vec3(-43.5285, -94.5454, -16.1499),
//glm::vec3(-46.6151, -94.5472, -9.92809),
//glm::vec3(-54.3819, -94.5464, -4.0909),
//glm::vec3(-73.1659, -94.5447, 11.8797),
//glm::vec3(-74.7481, -94.5462, 26.6788),
//glm::vec3(-66.3723, -91.2688, 33.3656),
//glm::vec3(-54.2916, -84.9851, 44.2097),
//glm::vec3(-42.0976, -84.5365, 45.9866),
//glm::vec3(-32.5076, -81.7406, 38.2317),
//glm::vec3(-13.8132, -73.2113, 19.9834),
//glm::vec3(2.87696, -71.0276, 9.99774),
//glm::vec3(4.5325, -71.1358, 11.3034),
//glm::vec3(5.22488, -71.1504, 12.1013),
//glm::vec3(10.2568, -71.4079, 20.5117),
//glm::vec3(17.5593, -70.1626, 34.567),
//glm::vec3(16.9669, -68.5499, 38.3694),
//glm::vec3(9.36336, -65.0321, 40.6979),
//glm::vec3(1.62152, -60.9418, 41.5452),
//glm::vec3(-4.93453, -59.6856, 44.5246),
//glm::vec3(-16.3894, -59.5465, 54.9809),
//glm::vec3(-31.0652, -59.5388, 73.0464),
//glm::vec3(-29.4427, -59.5478, 78.1079),
//glm::vec3(-16.604, -55.2639, 84.2534),
//glm::vec3(3.23928, -45.0491, 96.1096),
//glm::vec3(14.8546, -40.595, 89.8412),
//glm::vec3(20.1091, -39.5229, 83.5266),
//glm::vec3(26.6524, -38.3333, 73.0763),
//glm::vec3(24.6848, -38.2021, 60.4575),
//glm::vec3(25.9412, -37.9538, 54.832),
//glm::vec3(34.0655, -37.952, 47.9181),
//glm::vec3(39.2359, -36.6098, 49.7402),
//glm::vec3(53.8035, -26.4754, 60.0108),
//glm::vec3(75.098, -18.5528, 72.1487),
//glm::vec3(86.443, -19.4369, 76.1686),
//glm::vec3(74.891, -19.5463, 83.4253),
//glm::vec3(62.8742, -15.9536, 100.107),
//glm::vec3(44.3057, -10.2979, 111.62),
//glm::vec3(27.9747, -7.28955, 125.713),
	};

	int nBetween = 5;

	// Build trackWaypoints: include first anchor, then for each segment add nBetween interior points.
	if (anchors.empty())
		return;

	//for (int i = 0; i < anchors.size() - 1; ++i)
	//{
	//	Waypoint wp;
	//	wp.position = anchors[i];
	//	wp.recommendedSpeed = 12.0f;
	//	wp.trackWidth = 5.0f;
	//	trackWaypoints.push_back(wp);
	//}

	Waypoint firstWp;
	firstWp.position = anchors.front();
	firstWp.recommendedSpeed = 12.0f;
	firstWp.trackWidth = 5.0f;
	trackWaypoints.push_back(firstWp);
	
	for (size_t i = 0; i < anchors.size() - 1; ++i)
	{
		// push start anchor of this segment
	
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
		Waypoint endWp;
		endWp.position = anchors[i + 1];
		endWp.recommendedSpeed = 12.0f;
		endWp.trackWidth = 5.0f;
		trackWaypoints.push_back(endWp);
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