#include "AiSystem.h"
#include "../Components/Render.h"
#include "../Components/Transform.h"
#include "../Components/Physics.h"
#include "../Components/Player.h"
#include "../Components/AiDriver.h"
#include "../ECSController.h"
#include <cmath>
#include <iostream>
#include "../NavMesh.h"

extern ECSController controller;

//void AiSystem::Init()
//{
//	InitializeRaceAnchors();
//}
//
//void AiSystem::InitializeRaceAnchors()
//{
//	raceAnchors = {
//		glm::vec3(-39.0f, -94.0f, -8.0f),
//		glm::vec3(-80.0f, -93.0f, 19.0f),
//		glm::vec3(-49.266f, -84.591f, 49.396f),
//		glm::vec3(-5.0f, -70.691f, 5.0f),
//		glm::vec3(22.0f, -70.194f, 32.0f),
//		glm::vec3(-3.312f, -61.604f, 35.852f),
//		glm::vec3(-10.312f, -61.604f, 45.852f),
//		glm::vec3(-28.125f, -59.594f, 65.995f),
//		glm::vec3(4.254f, -43.482f, 101.625f),
//		glm::vec3(29.236f, -38.011f, 71.075f),
//		glm::vec3(21.665f, -39.062f, 56.706f),
//		glm::vec3(39.063f, -38.777f, 39.908f),
//		glm::vec3(81.944f, -19.593f, 81.963f),
//		glm::vec3(25.0f, -3.5f, 120.0f), // finish line
//	};
//
//	std::cout << "[AiSystem] Initialized " << raceAnchors.size() << " race anchors" << std::endl;
//}

void AiSystem::Init()
{
}

void AiSystem::SetNavMesh(const NavMesh& mesh)
{
	navMesh = mesh;
	std::cout << "[AiSystem] NavMesh set: " << navMesh.TriangleCount() << " triangles." << std::endl;
}

void AiSystem::ComputeNavPath(Entity entity)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	glm::vec3 startPos = transform.position;

	int32_t startTri = navMesh.FindTriangle(startPos);
	if (startTri < 0) startTri = navMesh.FindClosestTriangle(startPos);

	int32_t goalTri = navMesh.FindTriangle(goalPosition);
	if (goalTri < 0) goalTri = navMesh.FindClosestTriangle(goalPosition);

	std::cout << "[AiSystem] A* from tri " << startTri << " to tri " << goalTri << std::endl;

	NavPath path = navMesh.FindPath(startTri, goalTri, startPos, goalPosition);

	ai.navWaypoints = path.waypoints;
	ai.currentWaypointIndex = 0;

	if (ai.navWaypoints.size() > 1)
		ai.currentWaypointIndex = 1;

	std::cout << "[AiSystem] Computed A* path: " << path.triangleIndices.size()
		<< " corridor triangles -> " << ai.navWaypoints.size() << " waypoints" << std::endl;
}

void AiSystem::SpawnDebugWaypoints(Entity aiEntity)
{
	ComputeNavPath(aiEntity);

	auto& ai = controller.GetComponent<AiDriver>(aiEntity);

	std::cout << "[AiSystem] Spawning " << ai.navWaypoints.size() << " debug waypoint markers" << std::endl;

	for (size_t i = 0; i < ai.navWaypoints.size(); ++i)
	{
		Entity marker = controller.createEntity();
		controller.AddComponent(marker, PhysicsBody{});
		controller.AddComponent(marker, Transform{
			ai.navWaypoints[i],
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec3(0.25f)
			});
		controller.AddComponent(marker, CheckPoint{ glm::quat(1.0f, 0.0f, 0.0f, 0.0f) });
		controller.AddComponent(marker, Trigger{ nullptr, 1.0f, 4.0f, 1.0f });

		std::cout << "  marker[" << i << "]: (" << ai.navWaypoints[i].x << ", "
			<< ai.navWaypoints[i].y << ", " << ai.navWaypoints[i].z << ")" << std::endl;
	}
}

void AiSystem::Update(float deltaTime)
{
	for (auto entity : entities)
	{
		auto& ai = controller.GetComponent<AiDriver>(entity);

		if (ai.navWaypoints.empty() && !navMesh.IsEmpty())
		{
			ComputeNavPath(entity);
		}

		if (ai.navWaypoints.empty())
			continue;

		UpdateStateMachine(entity, deltaTime);
	}
}

void AiSystem::TransitionToState(Entity entity, AiState newState)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	ai.previousState = ai.currentState;
	ai.currentState = newState;

	switch (newState)
	{
	case AiState::BackingUp:
		ai.backupTimer = 0.0f;
		break;
	case AiState::FollowPath:
		ai.stuckTimer = 0.0f;
		break;
	default:
		break;
	}
}

void AiSystem::UpdateStateMachine(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);

	switch (ai.currentState)
	{
	case AiState::FollowPath:
		UpdateFollowPathState(entity, deltaTime);
		break;
	case AiState::BackingUp:
		UpdateBackingUpState(entity, deltaTime);
		break;
	case AiState::RecoveringFromOffTrack:
		UpdateRecoveringFromOffTrackState(entity, deltaTime);
		break;
	}
}

void AiSystem::UpdateFollowPathState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& body = controller.GetComponent<VehicleBody>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

	static float debugTimer = 0.0f;
	debugTimer += deltaTime;
	bool shouldLog = debugTimer >= 0.5f;
	if (shouldLog) debugTimer = 0.0f;

	if (ai.currentWaypointIndex >= static_cast<uint32_t>(ai.navWaypoints.size()))
	{
		if (shouldLog) std::cout << "[AI] Reached end of path, stopping." << std::endl;
		vc.throttle = 0.0f;
		vc.brake = 1.0f;
		vc.steer = 0.0f;
		vc.isGrounded = true;
		return;
	}

	glm::vec3 carPos = transform.position;
	glm::vec3 waypointPosition = ai.navWaypoints[ai.currentWaypointIndex];

	// Use XZ distance for arrival
	glm::vec3 toWpXZ = glm::vec3(waypointPosition.x - carPos.x, 0.0f, waypointPosition.z - carPos.z);
	float distXZ = glm::length(toWpXZ);

	// Advance through waypoints within arrival radius
	while (distXZ < ai.arrivalRadius &&
		ai.currentWaypointIndex + 1 < static_cast<uint32_t>(ai.navWaypoints.size()))
	{
		ai.currentWaypointIndex++;
		waypointPosition = ai.navWaypoints[ai.currentWaypointIndex];
		toWpXZ = glm::vec3(waypointPosition.x - carPos.x, 0.0f, waypointPosition.z - carPos.z);
		distXZ = glm::length(toWpXZ);
	}

	// Forward direction in XZ
	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	glm::vec3 toTargetN = distXZ > 1e-5f ? glm::normalize(toWpXZ) : forward;
	float forwardDot = glm::clamp(glm::dot(forward, toTargetN), -1.0f, 1.0f);

	// Only zero steer when almost perfectly aligned TOWARD the target
	// (not when facing away — that needs full steering correction)
	float steer = 0.0f;
	if (std::abs(forwardDot) > ai.steerDeadzoneDot)
	{
		steer = 0.0f; // Nearly aligned, go straight
	}
	else
	{
		steer = CalculateSteeringAngle(forward, toWpXZ);
	}

	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));
	float baseThrottle = glm::clamp((ai.desiredSpeed - speed) * ai.throttleKp, 0.0f, ai.maxThrottle);

	if (shouldLog)
	{
		std::cout << "[AI] pos=(" << carPos.x << ", " << carPos.y << ", " << carPos.z << ")"
			<< " | wp[" << ai.currentWaypointIndex << "/" << ai.navWaypoints.size() << "]=("
			<< waypointPosition.x << ", " << waypointPosition.y << ", " << waypointPosition.z << ")"
			<< " | distXZ=" << distXZ
			<< " | fwdDot=" << forwardDot
			<< " | steer=" << steer
			<< " | throttle=" << baseThrottle
			<< " | speed=" << speed
			<< std::endl;
	}

	// Stuck detection
	if (speed < ai.stuckSpeedThreshold && baseThrottle > 0.5f)
	{
		ai.stuckTimer += deltaTime;
		if (ai.stuckTimer > ai.stuckTimeThreshold)
		{
			std::cout << "[AI] STUCK — backing up" << std::endl;
			TransitionToState(entity, AiState::BackingUp);
			return;
		}
	}
	else
	{
		ai.stuckTimer = 0.0f;
	}

	vc.steer = steer;
	vc.throttle = baseThrottle;
	vc.brake = 0.0f;
	vc.isGrounded = true;
}

void AiSystem::UpdateBackingUpState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

	ai.backupTimer += deltaTime;

	if (ai.backupTimer >= ai.backupDuration)
	{
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	if (ai.currentWaypointIndex >= static_cast<uint32_t>(ai.navWaypoints.size()))
	{
		vc.steer = 0.0f;
		vc.throttle = 0.0f;
		vc.brake = 1.0f;
		vc.isGrounded = true;
		return;
	}

	glm::vec3 toTarget = ai.navWaypoints[ai.currentWaypointIndex] - transform.position;
	toTarget.y = 0.0f;

	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	glm::vec2 fwd2D(forward.x, forward.z);
	glm::vec2 target2D(toTarget.x, toTarget.z);
	if (glm::length(target2D) > 1e-5f)
		target2D = glm::normalize(target2D);

	float cross = fwd2D.x * target2D.y - fwd2D.y * target2D.x;
	float dot = glm::clamp(fwd2D.x * target2D.x + fwd2D.y * target2D.y, -1.0f, 1.0f);
	float angle = std::atan2(cross, dot);

	float steer = glm::clamp(angle * 1.0f, -1.0f, 1.0f);

	vc.steer = -steer;
	vc.throttle = 0.0f;
	vc.brake = 1.0f;
	vc.isGrounded = true;
}

float AiSystem::CalculateSteeringAngle(const glm::vec3& forward, const glm::vec3& toTarget)
{
	glm::vec2 fwd2D(forward.x, forward.z);
	glm::vec2 target2D(toTarget.x, toTarget.z);

	if (glm::length(target2D) < 1e-5f)
		return 0.0f;

	target2D = glm::normalize(target2D);

	float cross = fwd2D.x * target2D.y - fwd2D.y * target2D.x;
	float dot = glm::clamp(fwd2D.x * target2D.x + fwd2D.y * target2D.y, -1.0f, 1.0f);
	float angle = std::atan2(cross, dot);

	// Use gain of 1.0 so the AI gets full lock when needed
	return glm::clamp(-angle * 1.0f, -1.0f, 1.0f);
}

void AiSystem::UpdateRecoveringFromOffTrackState(Entity entity, float deltaTime)
{
	TransitionToState(entity, AiState::FollowPath);
}