#include "AiSystem.h"
#include "../Game.h"
#include "../Components/Render.h"
#include "../Components/Transform.h"
#include "../Components/Physics.h"
#include "../Components/Player.h"
#include "../Components/AiDriver.h"
#include "../Components/Powerup.h"
#include "../Components/Obstacle.h"
#include "../Components/Banana.h"
#include "../ECSController.h"
#include <cmath>
#include <iostream>
#include "../NavMesh.h"
#include "../Components/DangerZone.h"

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

	// Strategic waypoints for track navigation
	glm::vec3 beforeGap(-60.0f, -31.0f, 170.0f);
	glm::vec3 afterGap(135.0f, -31.0f, 185.0f);

	std::cout << "[AiSystem] Computing path from (" << startPos.x << ", " << startPos.y << ", " << startPos.z << ")" 
		<< " to waypoint before gap (" << beforeGap.x << ", " << beforeGap.y << ", " << beforeGap.z << ")" << std::endl;

	// For now: just path to the waypoint before the gap
	int32_t startTri = navMesh.FindTriangle(startPos);
	if (startTri < 0) startTri = navMesh.FindClosestTriangle(startPos);

	int32_t goalTri = navMesh.FindTriangle(beforeGap);
	if (goalTri < 0) goalTri = navMesh.FindClosestTriangle(beforeGap);

	std::cout << "[AiSystem] Start triangle: " << startTri << ", Goal triangle: " << goalTri << std::endl;

	NavPath path = navMesh.FindPath(startTri, goalTri, startPos, beforeGap);

	ai.navWaypoints = path.waypoints;
	ai.currentWaypointIndex = 0;

	if (ai.navWaypoints.size() > 1)
		ai.currentWaypointIndex = 1;

	std::cout << "[AiSystem] Path computed: " << path.triangleIndices.size()
		<< " corridor triangles -> " << ai.navWaypoints.size() << " waypoints" << std::endl;

	if (path.waypoints.empty())
	{
		std::cout << "[AiSystem] WARNING: Path computation returned 0 waypoints!" << std::endl;
	}
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

	static float debugTimer = 0.0f;
	debugTimer += deltaTime;
	bool shouldLog = debugTimer >= 0.5f;
	if (shouldLog) debugTimer = 0.0f;

	// --- Flip detection ---
	// Check if the vehicle's local up vector is pointing downward (flipped over)
	glm::vec3 vehicleUp = transform.quatRotation * glm::vec3(0.0f, 1.0f, 0.0f);
	if (vehicleUp.y < 0.1f) // up vector pointing sideways or downward
	{
		ai.flippedTimer += deltaTime;
		if (ai.flippedTimer > ai.flippedTimeThreshold)
		{
			std::cout << "[AI] Flipped over, resetting vehicle" << std::endl;
			ai.flippedTimer = 0.0f;

			Event resetEvent(Events::Player::RESET_VEHICLE);
			resetEvent.SetParam<Entity>(Events::Player::Reset_Vehicle::ENTITY, entity);
			controller.SendEvent(resetEvent);

			RecomputeNavPath(entity);
			TransitionToState(entity, AiState::FollowPath);
			return;
		}
	}
	else
	{
		ai.flippedTimer = 0.0f;
	}

	switch (ai.currentState)
	{
	case AiState::FollowPath:
	{
		// Off-track: check if car is far below the nearest navmesh surface
		int32_t nearestTri = navMesh.FindTriangleAtHeight(transform.position);
		if (nearestTri < 0)
			nearestTri = navMesh.FindClosestTriangleAtHeight(transform.position, 5.0f);

		bool offTrack = false;
		if (nearestTri >= 0)
		{
			float surfaceY = navMesh.GetTriangles()[nearestTri].centroid.y;
			float heightBelow = surfaceY - transform.position.y;
			if (heightBelow > ai.offTrackHeightThreshold)
			{
				std::cout << "[AI] Off-track (below navmesh surface)! heightBelow=" << heightBelow << std::endl;
				offTrack = true;
			}
		}
		else
		{
			offTrack = true;
			std::cout << "[AI] Off-track (no navmesh found nearby)" << std::endl;
		}

		if (offTrack && ai.repathCooldown <= 0.0f)
		{
			TransitionToState(entity, AiState::RecoveringFromOffTrack);
			break;
		}

		// Tick danger detection cooldown
		if (ai.dangerDetectionCooldown > 0.0f)
			ai.dangerDetectionCooldown -= deltaTime;

		// --- Danger zone check: stop if upcoming waypoints enter a danger zone ---
		if (!ai.passingThroughDangerZone && ai.dangerDetectionCooldown <= 0.0f)
		{
			bool pathEntersDanger = false;

			uint32_t lookAheadForDanger = ai.currentWaypointIndex + 3u;

for (uint32_t i = ai.currentWaypointIndex; i < lookAheadForDanger && i < static_cast<uint32_t>(ai.navWaypoints.size()); ++i)
			{
				if (IsPointInDangerZone(ai.navWaypoints[i]))
				{
					pathEntersDanger = true;
					break;
				}
			}

			if (pathEntersDanger)
			{
				if (shouldLog) {
					std::cout << "[AI] Danger ahead, entering waiting state" << std::endl;
				}
				TransitionToState(entity, AiState::WaitingAtDangerZone);
				break;
			}
		}
		// --- Obstacle detection: scan for MovingObstacles and Bananas ahead ---
		{
			glm::vec3 carPos = transform.position;
			glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
			forward.y = 0.0f;
			if (glm::length(forward) > 1e-6f) forward = glm::normalize(forward);

			Entity closestObstacle = 0;
			float closestDist = ai.obstacleDetectionRange;
			glm::vec3 closestObstaclePos(0.0f);

			static float obstacleDebugTimer = 0.0f;
			obstacleDebugTimer += deltaTime;
			bool logObstacles = obstacleDebugTimer >= 1.0f;
			if (logObstacles) obstacleDebugTimer = 0.0f;

			// Scan MovingObstacles
			//auto obstacleArray = controller.GetComponentArray<MovingObstacle>();
			//int obstacleCount = 0;
			//int obstaclesInRange = 0;
			//int obstaclesInCone = 0;

			//for (auto& [obsEntity, idx] : obstacleArray->GetEntityToIndexMap())
			//{
			//	if (!controller.HasComponent<Transform>(obsEntity))
			//		continue;

			//	obstacleCount++;
			//	auto& obsTransform = controller.GetComponent<Transform>(obsEntity);
			//	glm::vec3 toObs = obsTransform.position - carPos;
			//	toObs.y = 0.0f;
			//	float dist = glm::length(toObs);

			//	if (dist < 1e-5f || dist > closestDist)
			//	{
			//		if (logObstacles && dist <= ai.obstacleDetectionRange && dist > 1e-5f)
			//		{
			//			obstaclesInRange++;
			//		}
			//		continue;
			//	}

			//	obstaclesInRange++;

			//	float dot = glm::dot(forward, glm::normalize(toObs));
			//	if (dot > ai.obstacleDetectionCone)
			//	{
			//		obstaclesInCone++;
			//		closestDist = dist;
			//		closestObstacle = obsEntity;
			//		closestObstaclePos = obsTransform.position;
			//	}
			//}

			//if (logObstacles)
			//{
			//	std::cout << "[AI] Obstacle scan: " << obstacleCount << " total, "
			//		<< obstaclesInRange << " in range, " << obstaclesInCone << " in cone" << std::endl;
			//}

			// Scan Bananas
			auto bananaArray = controller.GetComponentArray<Banana>();
			for (auto& [banEntity, idx] : bananaArray->GetEntityToIndexMap())
			{
				if (!controller.HasComponent<Transform>(banEntity))
					continue;

				auto& banTransform = controller.GetComponent<Transform>(banEntity);
				glm::vec3 toBan = banTransform.position - carPos;
				toBan.y = 0.0f;
				float dist = glm::length(toBan);

				if (dist < 1e-5f || dist > closestDist)
					continue;

				float dot = glm::dot(forward, glm::normalize(toBan));
				if (dot > ai.obstacleDetectionCone)
				{
					closestDist = dist;
					closestObstacle = banEntity;
					closestObstaclePos = banTransform.position;
				}
			}

			if (closestObstacle != 0)
			{
				// Decide which direction to dodge: steer away from the obstacle
				// Use the cross product to determine if obstacle is left or right of us
				glm::vec3 toObs = closestObstaclePos - carPos;
				toObs.y = 0.0f;
				float cross = forward.x * toObs.z - forward.z * toObs.x;

				// If obstacle is to our right (cross > 0), steer left (-1), and vice versa
				ai.avoidanceSteerDirection = (cross > 0.0f) ? -1.0f : 1.0f;
				ai.detectedObstacleEntity = closestObstacle;
				ai.avoidTimer = 0.0f;

				std::cout << "[AI] Obstacle detected, dodging "
					<< (ai.avoidanceSteerDirection < 0.0f ? "left" : "right")
					<< " (dist=" << closestDist << ")" << std::endl;

				TransitionToState(entity, AiState::AvoidObstacle);
				break;
			}
		}


		// Check for nearby powerups (only if we don't already have one)
		if (!ai.hasPowerup)
		{
			Entity bestPowerup = 0;
			float bestDist = ai.powerupSeekRange;

			glm::vec3 carPos = transform.position;
			glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
			forward.y = 0.0f;
			if (glm::length(forward) > 1e-6f) forward = glm::normalize(forward);

			auto powerupArray = controller.GetComponentArray<Powerup>();
			for (auto& [powerupEntity, idx] : powerupArray->GetEntityToIndexMap())
			{
				if (!controller.HasComponent<Transform>(powerupEntity))
					continue;

				auto& pickup = controller.GetComponent<Powerup>(powerupEntity);
				if (pickup.active)
					continue;

				auto& powerupTransform = controller.GetComponent<Transform>(powerupEntity);
				glm::vec3 toPowerup = powerupTransform.position - carPos;
				toPowerup.y = 0.0f;

				float dist = glm::length(toPowerup);
				if (dist < 1e-5f || dist > bestDist)
					continue;

				float dot = glm::dot(forward, glm::normalize(toPowerup));
				if (dot < ai.powerupSeekMaxAngle)
					continue;

				bestDist = dist;
				bestPowerup = powerupEntity;
			}

			if (bestPowerup != 0)
			{
				ai.targetPowerupEntity = bestPowerup;
				ai.seekTimer = 0.0f;
				std::cout << "[AI] Spotted powerup, detouring to collect" << std::endl;
				TransitionToState(entity, AiState::SeekPowerup);
				break;
			}
		}

		// Try to use a held powerup (checked inline -- does NOT interrupt driving)
		if (ai.hasPowerup)
		{
			TryUsePowerup(entity);
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
	case AiState::WaitingAtDangerZone:
		UpdateWaitingAtDangerZoneState(entity, deltaTime);
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
	bool shouldLog = debugTimer >= 0.1f;
	if (shouldLog) debugTimer = 0.0f;

	// Clear passing flag the moment we exit the danger zone
	if (ai.passingThroughDangerZone)
	{
		if (!IsPointInDangerZone(transform.position))
		{
			ai.passingThroughDangerZone = false;
			ai.dangerDetectionCooldown = ai.dangerDetectionCooldownDuration;
			if (shouldLog) std::cout << "[AI] Exited danger zone, starting cooldown (" 
				<< ai.dangerDetectionCooldownDuration << "s)" << std::endl;
		}
	}

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

	// --- Waypoint advancement ---
	// Advance past waypoints that are either:
	// 1. Within arrival radius (normal case)
	// 2. Behind the car (overshoot -- car blew past at high speed)
	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	bool advanced = true;
	while (advanced && ai.currentWaypointIndex + 1 < static_cast<uint32_t>(ai.navWaypoints.size()))
	{
		advanced = false;

		// Check 1: within arrival radius
		if (distXZ < ai.arrivalRadius)
		{
			advanced = true;
		}

		// Check 2: waypoint is behind us AND the next waypoint is ahead of us
		// This catches high-speed overshoots without incorrectly skipping forward waypoints
		if (!advanced && ai.currentWaypointIndex + 1 < static_cast<uint32_t>(ai.navWaypoints.size()))
		{
			float dotToWp = glm::dot(forward, distXZ > 1e-5f ? glm::normalize(toWpXZ) : forward);

			if (dotToWp < 0.0f) // waypoint is behind us
			{
				// Only skip if the NEXT waypoint is more ahead than this one
				glm::vec3 nextWp = ai.navWaypoints[ai.currentWaypointIndex + 1];
				glm::vec3 toNextXZ = glm::vec3(nextWp.x - carPos.x, 0.0f, nextWp.z - carPos.z);
				float nextDist = glm::length(toNextXZ);
				float dotToNext = (nextDist > 1e-5f) ? glm::dot(forward, glm::normalize(toNextXZ)) : 0.0f;

				if (dotToNext > dotToWp)
				{
					advanced = true;
				}
			}
		}

		if (advanced)
		{
			ai.currentWaypointIndex++;
			waypointPosition = ai.navWaypoints[ai.currentWaypointIndex];
			toWpXZ = glm::vec3(waypointPosition.x - carPos.x, 0.0f, waypointPosition.z - carPos.z);
			distXZ = glm::length(toWpXZ);

			ai.progressTimer = 0.0f;
			ai.lastDistToWaypoint = distXZ;
		}
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

	// --- Steering: look-ahead point along the path ---
	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));

	// Dynamic lookahead: the faster we go, the further ahead we steer
	float lookahead = ai.lookaheadDistance + speed * 0.3f;

	// Walk along the waypoint path to find the lookahead target point
	glm::vec3 steerTarget = waypointPosition;
	{
		float remaining = lookahead;
		glm::vec3 from = carPos;

		for (uint32_t i = ai.currentWaypointIndex; i < static_cast<uint32_t>(ai.navWaypoints.size()); ++i)
		{
			glm::vec3 to = ai.navWaypoints[i];
			glm::vec3 seg = to - from;
			seg.y = 0.0f;
			float segLen = glm::length(seg);

			if (segLen < 1e-5f)
			{
				from = to;
				continue;
			}

			if (remaining <= segLen)
			{
				steerTarget = from + (seg / segLen) * remaining;
				break;
			}

			remaining -= segLen;
			from = to;
			steerTarget = to;
		}
	}

	glm::vec3 toSteerXZ = glm::vec3(steerTarget.x - carPos.x, 0.0f, steerTarget.z - carPos.z);
	float steerDist = glm::length(toSteerXZ);
	glm::vec3 toTargetN = steerDist > 1e-5f ? glm::normalize(toSteerXZ) : forward;
	float forwardDot = glm::clamp(glm::dot(forward, toTargetN), -1.0f, 1.0f);

	float steer = 0.0f;
	if (forwardDot > ai.steerDeadzoneDot)
	{
		steer = 0.0f;
	}
	else
	{
		steer = CalculateSteeringAngle(forward, toSteerXZ);
	}

	// --- Throttle output ---
	float throttle = 0.0f;

	if (speed < ai.desiredSpeed)
	{
		throttle = glm::clamp((ai.desiredSpeed - speed) * ai.throttleKp, 0.0f, ai.maxThrottle);
	}

	//if (shouldLog)
	//{
	//	std::cout << "[AI] pos=(" << carPos.x << ", " << carPos.y << ", " << carPos.z << ")"
	//		<< " | wp[" << ai.currentWaypointIndex << "/" << ai.navWaypoints.size() << "]=("
	//		<< waypointPosition.x << ", " << waypointPosition.y << ", " << waypointPosition.z << ")"
	//		<< " | distXZ=" << distXZ
	//		<< " | fwdDot=" << forwardDot
	//		<< " | steer=" << steer
	//		<< " | speed=" << speed
	//		<< " | throttle=" << throttle
	//		<< std::endl;
	//}

	// Stuck detection -- check if car wants to move but physically can't
	// Use desiredSpeed > 0 as intent, not throttle output (which gets scaled down)
	bool wantsToMove = (ai.currentWaypointIndex < static_cast<uint32_t>(ai.navWaypoints.size()));
	if (speed < ai.stuckSpeedThreshold && wantsToMove)
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
	vc.throttle = throttle;
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

	std::cout << "[AI] Backing up... steer=" << steer << std::endl;

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

void AiSystem::UpdateAvoidObstacleState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& body = controller.GetComponent<VehicleBody>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

	ai.avoidTimer += deltaTime;

	// Done avoiding -- return to path
	if (ai.avoidTimer > ai.avoidDuration)
	{
		std::cout << "[AI] Obstacle avoidance complete, resuming path" << std::endl;
		ai.detectedObstacleEntity = 0;
		RecomputeNavPath(entity);
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	// Calculate distance to obstacle
	float distToObstacle = 999.0f;
	bool obstacleStillThreat = false;

	std::cout << "[AI] Avoiding obstacle: timer=" << ai.avoidTimer << "s, entity=" << ai.detectedObstacleEntity << std::endl;

	if (ai.detectedObstacleEntity != 0 && controller.HasComponent<Transform>(ai.detectedObstacleEntity))
	{
		glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
		forward.y = 0.0f;
		if (glm::length(forward) > 1e-6f) forward = glm::normalize(forward);

		auto& obsTransform = controller.GetComponent<Transform>(ai.detectedObstacleEntity);
		glm::vec3 toObs = obsTransform.position - transform.position;
		toObs.y = 0.0f;
		distToObstacle = glm::length(toObs);

		std::cout << "[AI] Checking obstacle: dist=" << distToObstacle << std::endl;

		if (distToObstacle > 1e-5f)
		{
			float dot = glm::dot(forward, glm::normalize(toObs));
			// Obstacle is still ahead and in range
			if (dot > 0.0f && distToObstacle < ai.obstacleDetectionRange * 1.5f) {
				obstacleStillThreat = true;
				std::cout << "[AI] Obstacle check: dist=" << distToObstacle << ", dot=" << dot
					<< ", still threat=" << obstacleStillThreat << std::endl;
			}
		}
	}

	// If obstacle cleared or passed, resume path
	if (!obstacleStillThreat && ai.avoidTimer > 0.5f)
	{
		std::cout << "[AI] Obstacle cleared, resuming path" << std::endl;
		ai.detectedObstacleEntity = 0;
		RecomputeNavPath(entity);
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	// --- Cautious behavior: stop before obstacle, wait for it to clear ---
	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));
	float safeStoppingDistance = 10.0f;  // stop at least 10 units before obstacle
	float throttle = 0.0f;
	float brake = 0.0f;
	float steer = 0.0f;

	if (distToObstacle < safeStoppingDistance)
	{
		// STOP: we're too close to the obstacle
		if (speed > 0.5f)
		{
			brake = 1.0f;  // full brake
			throttle = 0.0f;
		}
		else
		{
			// Already stopped, just hold position
			brake = 1.0f;
			throttle = 0.0f;
		}
		steer = 0.0f;  // don't turn while stopped
	}
	else if (distToObstacle < safeStoppingDistance * 2.0f)
	{
		// SLOW APPROACH: obstacle is close but not immediate threat
		// Creep forward slowly to get closer
		float targetSpeed = 5.0f;  // crawl speed
		if (speed < targetSpeed)
		{
			throttle = 0.1f;  // gentle acceleration
			brake = 0.0f;
		}
		else
		{
			throttle = 0.0f;
			brake = 0.5f;  // light braking to maintain crawl speed
		}

		// Steer toward path while approaching
		if (ai.currentWaypointIndex < static_cast<uint32_t>(ai.navWaypoints.size()))
		{
			glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
			forward.y = 0.0f;
			if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
			forward = glm::normalize(forward);

			glm::vec3 toWp = ai.navWaypoints[ai.currentWaypointIndex] - transform.position;
			toWp.y = 0.0f;
			steer = CalculateSteeringAngle(forward, toWp);
		}
	}
	else
	{
		// BRAKE HARD: obstacle detected ahead but we have room to brake
		if (speed > 5.0f)
		{
			brake = 1.0f;
			throttle = 0.0f;
		}
		else
		{
			// Already slow, coast to a stop
			throttle = 0.0f;
			brake = 0.3f;
		}

		// Steer toward path while braking
		if (ai.currentWaypointIndex < static_cast<uint32_t>(ai.navWaypoints.size()))
		{
			glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
			forward.y = 0.0f;
			if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
			forward = glm::normalize(forward);

			glm::vec3 toWp = ai.navWaypoints[ai.currentWaypointIndex] - transform.position;
			toWp.y = 0.0f;
			steer = CalculateSteeringAngle(forward, toWp);
		}
	}

	vc.steer = steer;
	vc.throttle = throttle;
	vc.brake = brake;
	vc.isGrounded = true;
}

bool AiSystem::HasDangerZone(Entity obstacleEntity) const
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return false;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);
		if (dz.obstacleEntity == obstacleEntity)
			return true;
	}
	return false;
}

bool AiSystem::IsPointInActiveDangerZone(const glm::vec3& point) const
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// Check if the linked obstacle is currently extended (dangerous)
		if (dz.obstacleEntity != 0 && controller.HasComponent<MovingObstacle>(dz.obstacleEntity))
		{
			auto& obstacle = controller.GetComponent<MovingObstacle>(dz.obstacleEntity);

			// Path indices 0->1 = extending, 1->2 = holding extended
			// Path indices 2->3 = retracting
			// Only dangerous when extending or holding (indices 0 or 1)
			bool gloveIsOut = (obstacle.currentPathIndex <= 1);
			if (!gloveIsOut)
				continue; // glove is retracting, safe to pass
		}

		// Simple AABB point-in-box test (ignoring Y for a flat arena)
		glm::vec3 diff = glm::abs(point - dz.center);
		if (diff.x <= dz.halfExtents.x &&
			diff.y <= dz.halfExtents.y &&
			diff.z <= dz.halfExtents.z)
		{
			return true;
		}
	}
	return false;
}

bool AiSystem::IsPointInDangerZone(const glm::vec3& point) const
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return false;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// Simple AABB point-in-box test (no obstacle position check)
		glm::vec3 diff = glm::abs(point - dz.center);
		if (diff.x <= dz.halfExtents.x &&
			diff.y <= dz.halfExtents.y &&
			diff.z <= dz.halfExtents.z)
		{
			return true; // point is in A danger zone
		}
	}
	return false;
}

bool AiSystem::IsObstacleInDangerZone(const glm::vec3& point) const
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return false;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// First check if the POINT is in this danger zone (AABB test)
		glm::vec3 diff = glm::abs(point - dz.center);
		if (!(diff.x <= dz.halfExtents.x && diff.z <= dz.halfExtents.z))
			continue; // point not in this zone, skip it

		// Point IS in the zone, now check if the obstacle is physically present
		if (dz.obstacleEntity != 0 && controller.HasComponent<MovingObstacle>(dz.obstacleEntity))
		{
			auto& obstacle = controller.GetComponent<MovingObstacle>(dz.obstacleEntity);

			// Is the glove currently extended (blocking the zone)?
			// Path indices 0->1 = extending/extended (GLOVE IS THERE)
			// Path indices 2->3 = retracting/retracted (GLOVE IS GONE)
			bool gloveIsPhysicallyPresent = (obstacle.currentPathIndex <= 1);

			if (gloveIsPhysicallyPresent)
			{
				// Glove is blocking - return true without logging (caller will log if needed)
				return true; // YES, glove is blocking the zone!
			}
		}
	}

	return false; // either no zone found, or glove is retracted
}

void AiSystem::UpdateWaitingAtDangerZoneState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& body = controller.GetComponent<VehicleBody>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

	glm::vec3 carPos = transform.position;
	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);
	

	// Throttle logging to once per second
	static float logTimer = 0.0f;
	logTimer += deltaTime;
	bool shouldLog = (logTimer >= 1.0f);
	if (shouldLog) logTimer = 0.0f;
	bool currentlyBlocked = false;

	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));

	// 1) Get distance to the danger zone (for switching states)
	float distToZone = GetDistanceToDangerZone(carPos);

	// 2) Check if obstacle is blocking
	bool isBlocked = IsArmBlocking(carPos);

	// Likely need to be tweaked
	float SLOW_DISTANCE = 30.0f;
	float STOP_DISTANCE = 10.0f;

	// 3) Behaviour checks

	// 3.1) Far away so we want normal driving
	if (distToZone > SLOW_DISTANCE)
	{
		// Not sure if you need anything else here?
		vc.throttle = 1.0f;
		vc.brake = 0.0f;
	}
	// 3.2) Approaching the danger, we should slow down
	else if (distToZone > STOP_DISTANCE)
	{
		// Likely not correct, but hopefully the idea comes across
		vc.throttle = 0.0f;
		vc.brake = 0.5f;
	}
	else
		// 4) AT THE BOUNDARY OF DANGER!
	{
		// 4.1) Are we blocked?
		// yes
		if (isBlocked)
		{
			currentlyBlocked = true;
			// STOP and HOLD
			vc.throttle = 0.0f;
			vc.brake = 0.0f;
		}
		// no
		else
		{
			currentlyBlocked = false;
			if (shouldLog) std::cout << "[AI] Danger zone clear, resuming path" << std::endl;
			// GO
			vc.throttle = 1.0f;
			vc.brake = 0.0f;

			ai.passingThroughDangerZone = true; // flag to prevent re-entering this state immediately
			TransitionToState(entity, AiState::FollowPath);
			return;
		}
	}

	// Steer toward next waypoint ONLY if moving
	// When stopped or nearly stopped (speed < 2.0), hold steering straight to prevent spinning
	if (speed > 2.0f && ai.currentWaypointIndex < ai.navWaypoints.size())
	{
		glm::vec3 toWp = ai.navWaypoints[ai.currentWaypointIndex] - carPos;
		toWp.y = 0.0f;
		vc.steer = CalculateSteeringAngle(forward, toWp);
	}
	else
	{
		// Hold straight when stopped/slow or no waypoint
		vc.steer = 0.0f;
	}

	vc.isGrounded = true;
	return; // stay in this state, keep approaching
}

float AiSystem::GetDistanceToDangerZone(glm::vec3 point)
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return 999.0f; // no danger zones exist

	float minDist = 999.0f;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// Calculate AABB bounds
		glm::vec3 minBounds = dz.center - dz.halfExtents;
		glm::vec3 maxBounds = dz.center + dz.halfExtents;

		// Find closest point on AABB to the given point
		glm::vec3 closestPoint;
		closestPoint.x = glm::clamp(point.x, minBounds.x, maxBounds.x);
		closestPoint.y = glm::clamp(point.y, minBounds.y, maxBounds.y);
		closestPoint.z = glm::clamp(point.z, minBounds.z, maxBounds.z);

		// Calculate distance (ignoring Y for flat distance)
		glm::vec3 diff = point - closestPoint;
		//diff.y = 0.0f;
		float dist = glm::length(diff);

		if (dist < minDist)
			minDist = dist;
	}

	return minDist;
}

bool AiSystem::IsArmBlocking(glm::vec3 carPos)
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return false;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// Check if car is near this danger zone
		glm::vec3 diff = glm::abs(carPos - dz.center);
		if (diff.x > dz.halfExtents.x + 15.0f || diff.z > dz.halfExtents.z + 15.0f)
			continue; // too far from this zone

		// Check if the glove is physically extended
		if (dz.obstacleEntity != 0 && controller.HasComponent<MovingObstacle>(dz.obstacleEntity))
		{
			auto& obstacle = controller.GetComponent<MovingObstacle>(dz.obstacleEntity);
			if (obstacle.currentPathIndex < 3) // glove not fully retracted
			{
				return true;
			}
		}
	}

	return false;
}

void AiSystem::UpdateBrakingState(Entity entity, float deltaTime)
{
	// TODO: Slow down before sharp turns, then resume FollowPath.
	//
	// Possible approach:
	// - Check the angle between current forward and the direction to a waypoint
	//   brakingLookaheadWaypoints ahead
	// - If the dot product is below brakingAngleThreshold, apply brakes
	//   and reduce speed to brakingSpeed
	// - Once speed is low enough or the turn is passed, transition to FollowPath

	TransitionToState(entity, AiState::FollowPath);
}

void AiSystem::UpdateSeekPowerupState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& body = controller.GetComponent<VehicleBody>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

	ai.seekTimer += deltaTime;

	// Give up if taking too long
	if (ai.seekTimer > ai.seekTimeout)
	{
		std::cout << "[AI] Powerup seek timed out, resuming path" << std::endl;
		ai.targetPowerupEntity = 0;
		RecomputeNavPath(entity);
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	// Check if the powerup entity still exists
	if (!controller.HasComponent<Powerup>(ai.targetPowerupEntity) ||
		!controller.HasComponent<Transform>(ai.targetPowerupEntity))
	{
		std::cout << "[AI] Powerup gone, resuming path" << std::endl;
		ai.targetPowerupEntity = 0;
		RecomputeNavPath(entity);
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	glm::vec3 carPos = transform.position;
	auto& powerupTransform = controller.GetComponent<Transform>(ai.targetPowerupEntity);
	glm::vec3 powerupPos = powerupTransform.position;

	glm::vec3 toPowerupXZ = glm::vec3(powerupPos.x - carPos.x, 0.0f, powerupPos.z - carPos.z);
	float distXZ = glm::length(toPowerupXZ);

	// Check if we're out of range (went past it or it moved)
	if (distXZ > ai.powerupSeekRange * 1.5f)
	{
		std::cout << "[AI] Powerup too far, resuming path" << std::endl;
		ai.targetPowerupEntity = 0;
		RecomputeNavPath(entity);
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	// Close enough to "collect" -- the trigger system handles the actual pickup
	// but we also handle it here in case trigger doesn't fire for AI
	if (distXZ < ai.arrivalRadius)
	{
		auto& pickup = controller.GetComponent<Powerup>(ai.targetPowerupEntity);
		ai.hasPowerup = true;
		ai.heldPowerupType = pickup.type;
		std::cout << "[AI] Collected powerup type " << pickup.type << std::endl;
		controller.DestroyEntity(ai.targetPowerupEntity);
		ai.targetPowerupEntity = 0;

		// Re-path from current position since we detoured off the original path
		RecomputeNavPath(entity);
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	// Steer toward the powerup
	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	float steer = CalculateSteeringAngle(forward, toPowerupXZ);

	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));
	float throttle = glm::clamp((ai.desiredSpeed * 0.7f - speed) * ai.throttleKp, 0.0f, ai.maxThrottle);

	vc.steer = steer;
	vc.throttle = throttle;
	vc.brake = 0.0f;
	vc.isGrounded = true;
}

void AiSystem::TryUsePowerup(Entity entity)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	if (!ai.hasPowerup)
		return;

	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	if (ai.heldPowerupType == 1)
	{
		// Speed boost: use on straightaways
		if (ai.currentWaypointIndex < static_cast<uint32_t>(ai.navWaypoints.size()))
		{
			glm::vec3 toWp = ai.navWaypoints[ai.currentWaypointIndex] - transform.position;
			toWp.y = 0.0f;
			float dist = glm::length(toWp);

			if (dist > 1e-5f)
			{
				float dot = glm::dot(forward, glm::normalize(toWp));

				if (dot > ai.useBoostDot)
				{
					controller.AddComponent(entity, Powerup{ 1, true, 5.0f, 0.0f });
					ai.hasPowerup = false;
					ai.heldPowerupType = 0;
					std::cout << "[AI] Using speed boost on straightaway" << std::endl;
				}
			}
		}
	}
	else if (ai.heldPowerupType == 2)
	{
		// Banana peel: drop when the player is close behind
		Entity playerEntity = 0;
		auto playerArray = controller.GetComponentArray<PlayerController>();
		for (auto& [pEntity, idx] : playerArray->GetEntityToIndexMap())
		{
			if (controller.HasComponent<Transform>(pEntity))
			{
				playerEntity = pEntity;
				break;
			}
		}

		if (playerEntity != 0)
		{
			auto& playerTransform = controller.GetComponent<Transform>(playerEntity);
			glm::vec3 toPlayer = playerTransform.position - transform.position;
			toPlayer.y = 0.0f;
			float dist = glm::length(toPlayer);

			if (dist > 1e-5f && dist < ai.dropBananaPlayerRange)
			{
				float dot = glm::dot(forward, glm::normalize(toPlayer));

				if (dot < -0.3f)
				{
					if (gameInstance)
					{
						gameInstance->SpawnBananaPeel(entity);
						ai.hasPowerup = false;
						ai.heldPowerupType = 0;
						std::cout << "[AI] Dropped banana peel behind" << std::endl;
					}
				}
			}
		}
	}
}

void AiSystem::UpdateUsePowerupState(Entity entity, float deltaTime)
{
	// Powerup usage is now handled inline in FollowPath via TryUsePowerup.
	// If we somehow end up here, just go back to FollowPath.
	TransitionToState(entity, AiState::FollowPath);
}

void AiSystem::UpdateOvertakingState(Entity entity, float deltaTime)
{
	// TODO: Pass the player vehicle when close behind.
	//
	// Possible approach:
	// - Detect the player entity ("VehicleCommands" tag) within overtakeDetectionRange
	// - If player is ahead and in the car's forward cone, steer to one side
	//   using overtakeSteerOffset
	// - Once past the player (player is now behind), transition back to FollowPath

	TransitionToState(entity, AiState::FollowPath);
}