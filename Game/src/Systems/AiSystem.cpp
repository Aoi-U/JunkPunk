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

	int32_t nearestTri = navMesh.FindTriangle(currentPos);
	if (nearestTri < 0) nearestTri = navMesh.FindClosestTriangleAtHeight(currentPos, 5.0f);

	int32_t goalTri = navMesh.FindTriangle(goalPosition);
	if (goalTri < 0) goalTri = navMesh.FindClosestTriangle(goalPosition);

	NavPath path = navMesh.FindPath(nearestTri, goalTri, currentPos, goalPosition);

	if (path.waypoints.empty())
	{
		std::cout << "[AiSystem] Re-path failed! No path from current position to goal." << std::endl;
		return;
	}

	ai.navWaypoints = path.waypoints;

	// Find the first waypoint that is ahead of the car
	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	ai.currentWaypointIndex = 0;
	for (uint32_t i = 1; i < static_cast<uint32_t>(ai.navWaypoints.size()); ++i)
	{
		glm::vec3 toWp = ai.navWaypoints[i] - currentPos;
		toWp.y = 0.0f;
		float dist = glm::length(toWp);

		if (dist < 1e-5f)
			continue;

		float dot = glm::dot(forward, glm::normalize(toWp));

		if (dot > -0.3f && dist > ai.arrivalRadius)
		{
			ai.currentWaypointIndex = i;
			break;
		}
	}

	if (ai.currentWaypointIndex == 0 && ai.navWaypoints.size() > 1)
		ai.currentWaypointIndex = 1;

	// Reset progress tracking
	ai.progressTimer = 0.0f;
	ai.lastDistToWaypoint = 999999.0f;
	ai.repathCooldown = ai.repathCooldownDuration;

	std::cout << "[AiSystem] Re-pathed: " << path.triangleIndices.size()
		<< " corridor tris -> " << ai.navWaypoints.size()
		<< " waypoints, starting at wp[" << ai.currentWaypointIndex << "]" << std::endl;
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

void AiSystem::SpawnDebugNodes()
{
	if (navMesh.IsEmpty())
	{
		std::cout << "[AiSystem] No navmesh loaded, cannot spawn debug nodes." << std::endl;
		return;
	}

	const auto& triangles = navMesh.GetTriangles();

	std::cout << "[AiSystem] Spawning " << triangles.size() << " debug node markers (1x1 triggers at each centroid)" << std::endl;

	for (size_t i = 0; i < triangles.size(); ++i)
	{
		Entity marker = controller.createEntity();
		controller.AddComponent(marker, PhysicsBody{});
		controller.AddComponent(marker, Transform{
			triangles[i].centroid,
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec3(0.1f)
			});
		controller.AddComponent(marker, CheckPoint{ glm::quat(1.0f, 0.0f, 0.0f, 0.0f) });
		controller.AddComponent(marker, Trigger{ nullptr, 1.0f, 1.0f, 1.0f });
	}

	std::cout << "[AiSystem] Done spawning " << triangles.size() << " node markers." << std::endl;
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

	// Tick re-path cooldown in all states
	if (ai.repathCooldown > 0.0f)
		ai.repathCooldown -= deltaTime;

	switch (ai.currentState)
	{
	case AiState::FollowPath:
	{
		// Off-track: check if car is far below the nearest navmesh surface
		// NOT the waypoint -- waypoints can be on ramps above us
		int32_t nearestTri = navMesh.FindTriangleAtHeight(transform.position);
		if (nearestTri < 0)
			nearestTri = navMesh.FindClosestTriangleAtHeight(transform.position, 5.0f);

		bool offTrack = false;
		if (nearestTri >= 0)
		{
			float surfaceY = navMesh.GetTriangles()[nearestTri].centroid.y;
			float heightBelow = surfaceY - transform.position.y;
			// Only trigger if we're significantly below the nearest walkable surface at our XZ
			if (heightBelow > ai.offTrackHeightThreshold)
			{
				std::cout << "[AI] Off-track (below navmesh surface)! heightBelow=" << heightBelow << std::endl;
				offTrack = true;
			}
		}
		else
		{
			// Can't find any navmesh nearby at our height -- we're off the map
			offTrack = true;
			std::cout << "[AI] Off-track (no navmesh found nearby)" << std::endl;
		}

		if (offTrack && ai.repathCooldown <= 0.0f)
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

	// Tick the re-path cooldown
	if (ai.repathCooldown > 0.0f)
		ai.repathCooldown -= deltaTime;

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

	// Advance past waypoints within arrival radius
	while (distXZ < ai.arrivalRadius &&
		ai.currentWaypointIndex + 1 < static_cast<uint32_t>(ai.navWaypoints.size()))
	{
		ai.currentWaypointIndex++;
		waypointPosition = ai.navWaypoints[ai.currentWaypointIndex];
		toWpXZ = glm::vec3(waypointPosition.x - carPos.x, 0.0f, waypointPosition.z - carPos.z);
		distXZ = glm::length(toWpXZ);

		ai.progressTimer = 0.0f;
		ai.lastDistToWaypoint = distXZ;
	}

	// --- Progress detection ---
	float progressMargin = 0.5f;
	if (distXZ < ai.lastDistToWaypoint - progressMargin)
	{
		ai.progressTimer = 0.0f;
		ai.lastDistToWaypoint = distXZ;
	}
	else
	{
		ai.progressTimer += deltaTime;
	}

	// If no progress for too long, re-path (with cooldown)
	if (ai.progressTimer > ai.progressTimeThreshold && ai.repathCooldown <= 0.0f)
	{
		std::cout << "[AI] No progress toward wp[" << ai.currentWaypointIndex
			<< "], re-pathing from current position" << std::endl;
		RecomputeNavPath(entity);
		ai.repathCooldown = ai.repathCooldownDuration;
		return;
	}

	// --- Steering ---
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
			<< " | progress=" << ai.progressTimer
			<< std::endl;
	}

	// Stuck detection (zero speed against a wall)
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

	// On entry: re-path once
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

	glm::vec3 carPos = transform.position;
	glm::vec3 targetWp = ai.navWaypoints[ai.currentWaypointIndex];
	glm::vec3 toWpXZ = glm::vec3(targetWp.x - carPos.x, 0.0f, targetWp.z - carPos.z);
	float distXZ = glm::length(toWpXZ);

	// Check if we're making progress toward the waypoint
	bool makingProgress = distXZ < ai.lastDistToWaypoint - 0.3f;
	if (makingProgress)
		ai.lastDistToWaypoint = distXZ;

	// Resume FollowPath once we're close to a waypoint AND on the navmesh
	if (distXZ < ai.arrivalRadius * 2.0f)
	{
		// Verify we're actually on the navmesh before resuming
		int32_t triCheck = navMesh.FindTriangleAtHeight(carPos, 5.0f);
		if (triCheck >= 0)
		{
			std::cout << "[AI] Recovery complete, back on navmesh" << std::endl;
			ai.repathCooldown = ai.repathCooldownDuration;
			TransitionToState(entity, AiState::FollowPath);
			return;
		}
	}

	// Re-path if stuck for too long, but respect cooldown
	if (ai.recoveryTimer > 5.0f && ai.repathCooldown <= 0.0f)
	{
		std::cout << "[AI] Recovery timeout, re-pathing..." << std::endl;
		ai.recoveryTimer = 0.0f;
		RecomputeNavPath(entity);
		return;
	}

	// Drive toward the waypoint
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