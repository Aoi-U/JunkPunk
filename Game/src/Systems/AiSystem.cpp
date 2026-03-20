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

void AiSystem::Init()
{
	if (useAnchors)
	{
		InitializeRaceAnchors();
		std::cout << "[AiSystem] Using anchor-based pathfinding" << std::endl;
	}
	else
	{
		std::cout << "[AiSystem] Using pure A* pathfinding" << std::endl;
	}
}

void AiSystem::ComputeNavPath(Entity entity, size_t anchorStartIndex)
{
	if (useAnchors)
		ComputeNavPathWithAnchors(entity, anchorStartIndex);
	else
		ComputeNavPathWithoutAnchors(entity);
}

void AiSystem::InitializeRaceAnchors()
{
	raceAnchors = {
		glm::vec3(-39.0f, -94.0f, -8.0f),
		glm::vec3(-80.0f, -93.0f, 19.0f),
		glm::vec3(-45.266f, -84.591f, 55.396f),
		glm::vec3(5.0f, -70.691f, 5.0f),
		glm::vec3(28.0f, -70.194f, 28.0f),
		glm::vec3(-3.312f, -61.604f, 35.852f),
		glm::vec3(-10.312f, -61.604f, 45.852f),
		glm::vec3(-28.125f, -59.594f, 65.995f),
		glm::vec3(4.254f, -43.482f, 101.625f),
		glm::vec3(29.236f, -38.011f, 71.075f),
		glm::vec3(21.665f, -39.062f, 56.706f),
		glm::vec3(39.063f, -38.777f, 39.908f),
		glm::vec3(81.944f, -19.593f, 81.963f),
		glm::vec3(25.0f, -3.5f, 120.0f), // finish line
	};

	std::cout << "[AiSystem] Initialized " << raceAnchors.size() << " race anchors" << std::endl;
}

void AiSystem::SetNavMesh(const NavMesh& mesh)
{
	navMesh = mesh;
	std::cout << "[AiSystem] NavMesh set: " << navMesh.TriangleCount() << " triangles." << std::endl;
}

size_t AiSystem::FindNearestAnchorIndex(const glm::vec3& position) const
{
	size_t bestIndex = 0;
	float bestDistSq = std::numeric_limits<float>::max();

	for (size_t i = 0; i < raceAnchors.size(); ++i)
	{
		glm::vec3 diff = raceAnchors[i] - position;
		float distSq = glm::dot(diff, diff);
		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;
			bestIndex = i;
		}
	}

	return bestIndex;
}

// This uses anchor points to create a multi-segment path, which is more expensive to compute but allows for better control over the route.
void AiSystem::ComputeNavPathWithAnchors(Entity entity, size_t anchorStartIndex)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	if (raceAnchors.empty())
	{
		std::cout << "[AiSystem] No race anchors defined!" << std::endl;
		return;
	}

	// Clamp start index
	if (anchorStartIndex >= raceAnchors.size())
		anchorStartIndex = raceAnchors.size() - 1;

	ai.navWaypoints.clear();

	glm::vec3 currentPos = transform.position;

	for (size_t i = anchorStartIndex; i < raceAnchors.size(); ++i)
	{
		glm::vec3 segGoal = raceAnchors[i];

		int32_t startTri = navMesh.FindTriangle(currentPos);
		if (startTri < 0) startTri = navMesh.FindClosestTriangle(currentPos);

		int32_t goalTri = navMesh.FindTriangle(segGoal);
		if (goalTri < 0) goalTri = navMesh.FindClosestTriangle(segGoal);

		NavPath segPath = navMesh.FindPath(startTri, goalTri, currentPos, segGoal);

		if (segPath.waypoints.size() <= 2)
		{
			if (ai.navWaypoints.empty())
				ai.navWaypoints.push_back(currentPos);
			ai.navWaypoints.push_back(segGoal);
		}
		else
		{
			if (ai.navWaypoints.empty())
				ai.navWaypoints.push_back(segPath.waypoints[0]);

			for (size_t w = 1; w < segPath.waypoints.size() - 1; ++w)
			{
				ai.navWaypoints.push_back(segPath.waypoints[w]);
			}

			ai.navWaypoints.push_back(segGoal);
		}

		std::cout << "[AiSystem] Segment " << i << ": " << segPath.triangleIndices.size()
			<< " corridor tris, " << segPath.waypoints.size() << " raw waypoints" << std::endl;

		currentPos = segGoal;
	}

	ai.currentWaypointIndex = 0;
	if (ai.navWaypoints.size() > 1)
		ai.currentWaypointIndex = 1;

	std::cout << "[AiSystem] Total path: " << ai.navWaypoints.size() << " waypoints (from anchor "
		<< anchorStartIndex << " to " << raceAnchors.size() - 1 << ")" << std::endl;
}

// This is pure A* from current position to goal, without using anchors. Cheaper to compute but less control over the route.
void AiSystem::ComputeNavPathWithoutAnchors(Entity entity)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	glm::vec3 startPos = transform.position;
	glm::vec3 goalPos(25.0f, -3.5f, 120.0f); // finish line — update this for the new map

	int32_t startTri = navMesh.FindTriangle(startPos);
	if (startTri < 0) startTri = navMesh.FindClosestTriangle(startPos);

	int32_t goalTri = navMesh.FindTriangle(goalPos);
	if (goalTri < 0) goalTri = navMesh.FindClosestTriangle(goalPos);

	std::cout << "[AiSystem] A* from tri " << startTri << " to tri " << goalTri << std::endl;

	NavPath path = navMesh.FindPath(startTri, goalTri, startPos, goalPos);

	ai.navWaypoints = path.waypoints;
	ai.currentWaypointIndex = 0;

	if (ai.navWaypoints.size() > 1)
		ai.currentWaypointIndex = 1;

	std::cout << "[AiSystem] A* path: " << path.triangleIndices.size()
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
	case AiState::RecoveringFromOffTrack:
		ai.recoveryTimer = 0.0f;
		break;
	default:
		break;
	}
}

void AiSystem::UpdateStateMachine(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	switch (ai.currentState)
	{
	case AiState::FollowPath:
	{
		// Off-track detection
		int32_t currentTri = navMesh.FindTriangle(transform.position);
		if (currentTri < 0 && ai.currentWaypointIndex < static_cast<uint32_t>(ai.navWaypoints.size()))
		{
			float dist = glm::length(ai.navWaypoints[ai.currentWaypointIndex] - transform.position);
			if (dist > ai.maxDistanceFromTrack)
			{
				std::cout << "[AI] Off-track! dist=" << dist << std::endl;
				TransitionToState(entity, AiState::RecoveringFromOffTrack);
				break;
			}
		}
		UpdateFollowPathState(entity, deltaTime);
		break;
	}
	case AiState::BackingUp:
		UpdateBackingUpState(entity, deltaTime);
		break;
	case AiState::RecoveringFromOffTrack:
		UpdateRecoveringFromOffTrackState(entity, deltaTime);
		break;
	case AiState::AvoidObstacle:
		UpdateAvoidObstacleState(entity, deltaTime);
		break;
	case AiState::Braking:
		UpdateBrakingState(entity, deltaTime);
		break;
	case AiState::SeekPowerup:
		UpdateSeekPowerupState(entity, deltaTime);
		break;
	case AiState::UsePowerup:
		UpdateUsePowerupState(entity, deltaTime);
		break;
	case AiState::Overtaking:
		UpdateOvertakingState(entity, deltaTime);
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

	glm::vec3 toWpXZ = glm::vec3(waypointPosition.x - carPos.x, 0.0f, waypointPosition.z - carPos.z);
	float distXZ = glm::length(toWpXZ);

	while (distXZ < ai.arrivalRadius &&
		ai.currentWaypointIndex + 1 < static_cast<uint32_t>(ai.navWaypoints.size()))
	{
		ai.currentWaypointIndex++;
		waypointPosition = ai.navWaypoints[ai.currentWaypointIndex];
		toWpXZ = glm::vec3(waypointPosition.x - carPos.x, 0.0f, waypointPosition.z - carPos.z);
		distXZ = glm::length(toWpXZ);
	}

	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	glm::vec3 toTargetN = distXZ > 1e-5f ? glm::normalize(toWpXZ) : forward;
	float forwardDot = glm::clamp(glm::dot(forward, toTargetN), -1.0f, 1.0f);

	float steer = 0.0f;
	if (forwardDot > ai.steerDeadzoneDot)
	{
		steer = 0.0f;
	}
	else
	{
		steer = CalculateSteeringAngle(forward, toWpXZ);
	}

	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));
	float baseThrottle = glm::clamp((ai.desiredSpeed - speed) * ai.throttleKp, 0.0f, ai.maxThrottle);

	//if (shouldLog)
	//{
	//	std::cout << "[AI] pos=(" << carPos.x << ", " << carPos.y << ", " << carPos.z << ")"
	//		<< " | wp[" << ai.currentWaypointIndex << "/" << ai.navWaypoints.size() << "]=("
	//		<< waypointPosition.x << ", " << waypointPosition.y << ", " << waypointPosition.z << ")"
	//		<< " | distXZ=" << distXZ
	//		<< " | fwdDot=" << forwardDot
	//		<< " | steer=" << steer
	//		<< " | throttle=" << baseThrottle
	//		<< " | speed=" << speed
	//		<< std::endl;
	//}

	if (speed < ai.stuckSpeedThreshold && baseThrottle > 0.5f)
	{
		ai.stuckTimer += deltaTime;
		if (ai.stuckTimer > ai.stuckTimeThreshold)
		{
			std::cout << "[AI] STUCK: backing up" << std::endl;
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

	float steer = glm::clamp(-angle * 1.0f, -1.0f, 1.0f);

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

	return glm::clamp(-angle * 1.0f, -1.0f, 1.0f);
}

void AiSystem::UpdateRecoveringFromOffTrackState(Entity entity, float deltaTime)
{
	std::cout << "Off track: updating recovery state..." << std::endl;
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

	ai.recoveryTimer += deltaTime;

	// Step 1: Find the nearest anchor ahead of our position and recompute the path
	// This only runs once on entry (recoveryTimer just started)
	if (ai.recoveryTimer < deltaTime * 1.5f)
	{
		size_t nearestAnchor = FindNearestAnchorIndex(transform.position);
		std::cout << "[AI] Recovery: recomputing path from nearest anchor " << nearestAnchor
			<< " (" << raceAnchors[nearestAnchor].x << ", "
			<< raceAnchors[nearestAnchor].y << ", "
			<< raceAnchors[nearestAnchor].z << ")" << std::endl;

		ComputeNavPath(entity, nearestAnchor);
	}

	// Step 2: Drive toward the first new waypoint
	if (ai.navWaypoints.empty())
	{
		// Path computation failed — just go back to FollowPath and hope for the best
		std::cout << "[AI] Recovery: no path found, returning to FollowPath" << std::endl;
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	glm::vec3 carPos = transform.position;
	glm::vec3 targetWp = ai.navWaypoints[ai.currentWaypointIndex];
	glm::vec3 toWpXZ = glm::vec3(targetWp.x - carPos.x, 0.0f, targetWp.z - carPos.z);
	float distXZ = glm::length(toWpXZ);

	// Once we're close to the first waypoint on the new path, we're back on track
	if (distXZ < ai.arrivalRadius * 2.0f)
	{
		std::cout << "[AI] Recovery complete, resuming FollowPath" << std::endl;
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	// Safety timeout — if we can't reach the path in 5 seconds, recompute again
	if (ai.recoveryTimer > 5.0f)
	{
		std::cout << "[AI] Recovery timeout, recomputing..." << std::endl;
		ai.recoveryTimer = 0.0f;
		size_t nearestAnchor = FindNearestAnchorIndex(transform.position);
		ComputeNavPath(entity, nearestAnchor);
		return;
	}

	// Drive toward the first waypoint on the new path
	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	float steer = CalculateSteeringAngle(forward, toWpXZ);

	vc.steer = steer;
	vc.throttle = ai.maxThrottle * 0.5f; // drive cautiously during recovery
	vc.brake = 0.0f;
	vc.isGrounded = true;
}

void AiSystem::UpdateAvoidObstacleState(Entity entity, float deltaTime)
{
	// TODO: Detect MovingObstacle and Banana entities in front of the car.
	//       Steer left or right to dodge, then return to FollowPath.
	//
	// Suggested approach:
	// - Check all entities with MovingObstacle or Banana components
	// - If any are within obstacleDetectionRange and in the car's forward cone
	// - Steer perpendicular to the obstacle direction
	// - Once obstacle is no longer ahead, transition back to FollowPath

	TransitionToState(entity, AiState::FollowPath);
}

void AiSystem::UpdateBrakingState(Entity entity, float deltaTime)
{
	// TODO: Slow down before sharp turns, then resume FollowPath.
	//
	// Suggested approach:
	// - Check the angle between current forward and the direction to a waypoint
	//   brakingLookaheadWaypoints ahead
	// - If the dot product is below brakingAngleThreshold, apply brakes
	//   and reduce speed to brakingSpeed
	// - Once speed is low enough or the turn is passed, transition to FollowPath

	TransitionToState(entity, AiState::FollowPath);
}

void AiSystem::UpdateSeekPowerupState(Entity entity, float deltaTime)
{
	// TODO: Briefly detour to collect a nearby powerup pickup.
	//
	// Suggested approach:
	// - Scan for entities with a Powerup component within powerupSeekRange
	// - Steer toward the nearest one
	// - Once collected (component removed by physics trigger) or out of range,
	//   transition back to FollowPath

	TransitionToState(entity, AiState::FollowPath);
}

void AiSystem::UpdateUsePowerupState(Entity entity, float deltaTime)
{
	// TODO: Decide when to activate a held powerup.
	//
	// Suggested approach:
	// - If holding a speed boost (type 1): activate on straightaways (high fwdDot)
	// - If holding a banana peel (type 2): drop when the player is behind and close
	// - After using, transition back to FollowPath

	TransitionToState(entity, AiState::FollowPath);
}

void AiSystem::UpdateOvertakingState(Entity entity, float deltaTime)
{
	// TODO: Pass the player vehicle when close behind.
	//
	// Suggested approach:
	// - Detect the player entity ("VehicleCommands" tag) within overtakeDetectionRange
	// - If player is ahead and in the car's forward cone, steer to one side
	//   using overtakeSteerOffset
	// - Once past the player (player is now behind), transition back to FollowPath

	TransitionToState(entity, AiState::FollowPath);
}