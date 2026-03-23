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
}

void AiSystem::SetNavMesh(const NavMesh& mesh)
{
	navMesh = mesh;
	std::cout << "[AiSystem] NavMesh set: " << navMesh.TriangleCount() << " triangles ("
		<< navMesh.TriangleCount() << " nodes available for pathfinding)" << std::endl;
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

	NavPath path = navMesh.FindPath(startTri, goalTri, startPos, goalPosition);

	ai.navWaypoints = path.waypoints;
	ai.currentWaypointIndex = 0;

	if (ai.navWaypoints.size() > 1)
		ai.currentWaypointIndex = 1;

	std::cout << "[AiSystem] Path computed: " << path.triangleIndices.size()
		<< " corridor triangles -> " << ai.navWaypoints.size() << " waypoints" << std::endl;
}

void AiSystem::RecomputeNavPath(Entity entity)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	glm::vec3 currentPos = transform.position;

	// Find the closest node (triangle centroid) to our current position
	int32_t nearestTri = navMesh.FindTriangle(currentPos);
	if (nearestTri < 0) nearestTri = navMesh.FindClosestTriangle(currentPos);

	int32_t goalTri = navMesh.FindTriangle(goalPosition);
	if (goalTri < 0) goalTri = navMesh.FindClosestTriangle(goalPosition);

	NavPath path = navMesh.FindPath(nearestTri, goalTri, currentPos, goalPosition);

	if (path.waypoints.empty())
	{
		std::cout << "[AiSystem] Re-path failed! No path from current position to goal." << std::endl;
		return;
	}

	ai.navWaypoints = path.waypoints;
	ai.currentWaypointIndex = 0;

	if (ai.navWaypoints.size() > 1)
		ai.currentWaypointIndex = 1;

	std::cout << "[AiSystem] Re-pathed: " << path.triangleIndices.size()
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
		bool offTrack = false;

		if (ai.currentWaypointIndex < static_cast<uint32_t>(ai.navWaypoints.size()))
		{
			glm::vec3 carPos = transform.position;
			glm::vec3 wpPos = ai.navWaypoints[ai.currentWaypointIndex];

			// Check 1: Car is below the current waypoint (fell off a cliff/ramp)
			float heightBelow = wpPos.y - carPos.y;
			if (heightBelow > ai.offTrackHeightThreshold)
			{
				std::cout << "[AI] Off-track (fell below waypoint)! heightBelow=" << heightBelow << std::endl;
				offTrack = true;
			}

			// Check 2: 3D distance is too large (knocked far away)
			if (!offTrack)
			{
				float dist3D = glm::length(wpPos - carPos);
				if (dist3D > ai.maxDistanceFromTrack)
				{
					std::cout << "[AI] Off-track (too far in 3D)! dist=" << dist3D << std::endl;
					offTrack = true;
				}
			}
		}

		if (offTrack)
		{
			TransitionToState(entity, AiState::RecoveringFromOffTrack);
			break;
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

	if (speed < ai.stuckSpeedThreshold && baseThrottle > 0.5f)
	{
		ai.stuckTimer += deltaTime;
		if (ai.stuckTimer > ai.stuckTimeThreshold)
		{
			std::cout << "[AI] STUCK - backing up" << std::endl;
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

void AiSystem::UpdateRecoveringFromOffTrackState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

	ai.recoveryTimer += deltaTime;

	// On entry: find the nearest node and recompute entire path to goal
	if (ai.recoveryTimer < deltaTime * 1.5f)
	{
		std::cout << "[AI] Recovery: finding nearest node and re-pathing to goal..." << std::endl;
		RecomputeNavPath(entity);
	}

	if (ai.navWaypoints.empty())
	{
		std::cout << "[AI] Recovery: no path found, returning to FollowPath" << std::endl;
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	// Drive toward the first waypoint on the new path
	glm::vec3 carPos = transform.position;
	glm::vec3 targetWp = ai.navWaypoints[ai.currentWaypointIndex];
	glm::vec3 toWpXZ = glm::vec3(targetWp.x - carPos.x, 0.0f, targetWp.z - carPos.z);
	float distXZ = glm::length(toWpXZ);

	// Once close to the new path, resume normal following
	if (distXZ < ai.arrivalRadius * 2.0f)
	{
		std::cout << "[AI] Recovery complete, resuming FollowPath" << std::endl;
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	// Safety timeout — re-path again if we can't reach it in 5 seconds
	if (ai.recoveryTimer > 5.0f)
	{
		std::cout << "[AI] Recovery timeout, re-pathing..." << std::endl;
		ai.recoveryTimer = 0.0f;
		RecomputeNavPath(entity);
		return;
	}

	// Steer toward the first waypoint
	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	float steer = CalculateSteeringAngle(forward, toWpXZ);

	vc.steer = steer;
	vc.throttle = ai.maxThrottle * 0.5f;
	vc.brake = 0.0f;
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

// ---- Stubs for future states ----

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