#include "AiSystem.h"
#include "AiSystemHelperFunctions.h"
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
	//std::cout << "[AiSystem] NavMesh set: " << navMesh.TriangleCount() << " triangles ("
	//	<< navMesh.TriangleCount() << " nodes available for pathfinding)" << std::endl;
}

void AiSystem::ComputeNavPath(Entity entity)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	glm::vec3 startPos = transform.position;
	glm::vec3 beforeGap(-60.0f, -31.0f, 170.0f);
	glm::vec3 afterGap(150.0f, -28.0f, 185.0f);
	glm::vec3 beforeTunnel(150.0f, -31.0f, 244.0f);
	glm::vec3 midTunnel(-135.600f, -28.000f, 392.500f);
	glm::vec3 inTunnel(-62.000f, -26.000f, 314.000f);
	glm::vec3 uppertrack(107.854f, 54.011f, 318.240f);

	// SEGMENT 1: Start -> beforeGap
	int32_t startTri = navMesh.FindTriangle(startPos);
	if (startTri < 0) startTri = navMesh.FindClosestTriangle(startPos);
	int32_t beforeGapTri = navMesh.FindTriangle(beforeGap);
	if (beforeGapTri < 0) beforeGapTri = navMesh.FindClosestTriangle(beforeGap);
	NavPath path1 = navMesh.FindPath(startTri, beforeGapTri, startPos, beforeGap);

	// SEGMENT 2: After gap -> before tunnel
	// Use FindTriangleAtHeight to avoid matching ceiling/wall triangles
	int32_t afterGapTri = navMesh.FindTriangleAtHeight(afterGap, 5.0f);
	if (afterGapTri < 0) afterGapTri = navMesh.FindClosestTriangleAtHeight(afterGap, 5.0f);

	int32_t beforeTunnelTri = navMesh.FindTriangleAtHeight(beforeTunnel, 5.0f);
	if (beforeTunnelTri < 0) beforeTunnelTri = navMesh.FindClosestTriangleAtHeight(beforeTunnel, 5.0f);
	NavPath path2 = navMesh.FindPath(afterGapTri, beforeTunnelTri, afterGap, beforeTunnel);

	int32_t midTunnelTri = navMesh.FindTriangleAtHeight(midTunnel, 5.0f);
	if (midTunnelTri < 0) midTunnelTri = navMesh.FindClosestTriangleAtHeight(midTunnel, 5.0f);
	NavPath path3 = navMesh.FindPath(beforeTunnelTri, midTunnelTri, beforeTunnel, midTunnel);

	// SEGMENT 3: Before tunnel -> inside tunnel
	int32_t inTunnelTri = navMesh.FindTriangle(inTunnel);
	if (inTunnelTri < 0) inTunnelTri = navMesh.FindClosestTriangle(inTunnel);
	NavPath path4 = navMesh.FindPath(midTunnelTri, inTunnelTri, midTunnel, inTunnel);

	// SEGMENT 5: Inside tunnel -> upper track
	int32_t upperTrackTri = navMesh.FindTriangle(uppertrack);
	if (upperTrackTri < 0) upperTrackTri = navMesh.FindClosestTriangle(uppertrack);
	NavPath path5 = navMesh.FindPath(inTunnelTri, upperTrackTri, inTunnel, uppertrack);

	// SEGMENT 6: Upper track -> goal
	int32_t goalTri = navMesh.FindTriangle(goalPosition);
	//std::cout << "[AiSystem] Segment 4: Goal position triangle using FindTriangle = " << goalTri << std::endl;
	if (goalTri < 0) goalTri = navMesh.FindClosestTriangle(goalPosition);
	//std::cout << "[AiSystem] Segment 4: Goal position triangle using FindClosestTriangle = " << goalTri << std::endl;

	NavPath path6 = navMesh.FindPath(upperTrackTri, goalTri, uppertrack, goalPosition);

	// Assemble the full waypoint path from all segments
	ai.navWaypoints = path1.waypoints;

	// Manual gap crossing
	ai.navWaypoints.push_back(beforeGap);  // approach platform
	ai.navWaypoints.push_back(afterGap);   // exit platform

	// Segment 2 (afterGap -> beforeTunnel)
	ai.navWaypoints.insert(ai.navWaypoints.end(),
		path2.waypoints.begin(),
		path2.waypoints.end());

	ai.navWaypoints.push_back(beforeTunnel);

	// Segment 3 (beforeTunnel -> midTunnel)
	if (!path3.waypoints.empty())
	{
		ai.navWaypoints.insert(ai.navWaypoints.end(),
			path3.waypoints.begin(),
			path3.waypoints.end());
	}
	else
	{
		ai.navWaypoints.push_back(midTunnel);  // Only if A* failed
	}

	// Segment 4: midTunnel -> inTunnel
	if (!path4.waypoints.empty())
	{
		ai.navWaypoints.insert(ai.navWaypoints.end(),
			path4.waypoints.begin(),
			path4.waypoints.end());
	}
	else
	{
		ai.navWaypoints.push_back(inTunnel);  // Only if A* failed
	}

	// Segment 4 (inside tunnel -> upper track)
	if (!path5.waypoints.empty())
	{
		ai.navWaypoints.insert(ai.navWaypoints.end(),
			path5.waypoints.begin(),
			path5.waypoints.end());
	}
	else
	{
		ai.navWaypoints.push_back(uppertrack);  // Only if A* failed
	}

	// Bridge to Segment 5
	ai.navWaypoints.push_back(uppertrack);

	// Segment 5 (uppertrack -> goal)
	ai.navWaypoints.insert(ai.navWaypoints.end(),
		path6.waypoints.begin(),
		path6.waypoints.end());

	ai.currentWaypointIndex = 0;
	if (ai.navWaypoints.size() > 1)
		ai.currentWaypointIndex = 1;
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

	//std::cout << "[AiSystem] Re-pathed: " << path.triangleIndices.size()
	//	<< " corridor tris -> " << ai.navWaypoints.size()
	//	<< " waypoints, starting at wp[" << ai.currentWaypointIndex << "]" << std::endl;
}

void AiSystem::Update(float deltaTime)
{
	for (auto entity : entities)
	{
		auto& ai = controller.GetComponent<AiDriver>(entity);
		auto& transform = controller.GetComponent<Transform>(entity);

		if (AiSystemHelperFunctions::ShouldLog(m_followPathLogTimer, 1.0f, deltaTime))
		{
			std::cout << "[AiSystem::Update] Entity=" << entity
				<< " | State=" << static_cast<int>(ai.currentState)
				<< " | Waypoints=" << ai.navWaypoints.size()
				<< " | CurrentWP=" << ai.currentWaypointIndex
				<< " | Pos=(" << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << ")"
				<< std::endl;
		}

		if (ai.navWaypoints.empty() && !navMesh.IsEmpty())
		{
			ComputeNavPath(entity);
		}

		if (ai.navWaypoints.empty())
			continue;

		switch (ai.currentState)
		{
		case AiState::FollowPath:
			UpdateFollowPathState(entity, deltaTime);
			break;
		case AiState::BackingUp:
			UpdateBackingUpState(entity, deltaTime);
			break;
		case AiState::IsStuck:
			UpdateIsStuckState(entity, deltaTime);
			break;
		case AiState::IsFlipped:
			UpdateIsFlippedState(entity, deltaTime);
			break;
		case AiState::RecoveringFromOffTrack:
			UpdateRecoveringFromOffTrackState(entity, deltaTime);
			break;
		case AiState::WaitingAtDangerZone:
			UpdateWaitingAtDangerZoneState(entity, deltaTime);
			break;
		case AiState::BoxingGloveZone:
			UpdateBoxingGloveZoneState(entity, deltaTime);
			break;
		case AiState::GapZone:
			UpdateGapZoneState(entity, deltaTime);
			break;
		case AiState::TunnelZone:
			UpdateTunnelZoneState(entity, deltaTime);
			break;
		case AiState::AvoidObstacle:
			UpdateAvoidObstacleState(entity, deltaTime);
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
		case AiState::Braking:
			UpdateBrakingState(entity, deltaTime);
			break;
		default:
			break;
		}
	}
}

void AiSystem::TransitionToState(Entity entity, AiState newState)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);

	// Early exit if already in this state
	if (ai.currentState == newState)
		return;

	ai.previousState = ai.currentState;
	ai.currentState = newState;

	// Reset timers when entering specific states
	switch (newState)
	{
	case AiState::FollowPath:
		ai.stuckTimer = 0.0f;
		break;
	case AiState::BackingUp:
		ai.backupTimer = 0.0f;
		break;
	case AiState::RecoveringFromOffTrack:
		ai.recoveryTimer = 0.0f;
		break;
	case AiState::IsStuck:
		ai.stuckTimer = 0.0f;
		break;
	case AiState::IsFlipped:
		ai.flippedTimer = 0.0f;
		break;
	default:
		break;
	}
	// NO update function calls here - prevents recursion!
}

void AiSystem::UpdateFollowPathState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& body = controller.GetComponent<VehicleBody>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

	glm::vec3 carPos = transform.position;
	glm::vec3 waypointPosition = ai.navWaypoints[ai.currentWaypointIndex];

	glm::vec3 toWpXZ = glm::vec3(waypointPosition.x - carPos.x, 0.0f, waypointPosition.z - carPos.z);
	float distXZ = glm::length(toWpXZ);

	//if (ShouldLog(m_followPathLogTimer, 1.0f, deltaTime)) {
	//	std::cout << "[FollowPath] Target WP[" << ai.currentWaypointIndex << "]=("
	//		<< waypointPosition.x << ", " << waypointPosition.y << ", " << waypointPosition.z
	//		<< ") | Dist=" << distXZ << std::endl;
	//}

	////////////////////////////////////////////////////////////////////////
	// --- Check if we've reached the end of the path, eg Finish line --- //
	////////////////////////////////////////////////////////////////////////
	if (ai.currentWaypointIndex >= static_cast<uint32_t>(ai.navWaypoints.size()))
	{
		if (AiSystemHelperFunctions::ShouldLog(m_followPathLogTimer, 0.1f, deltaTime)) {
				std::cout << "[AI] Reached end of path, stopping." << std::endl;
			}
		vc.throttle = 0.0f;
		vc.brake = 1.0f;
		vc.steer = 0.0f;
		vc.isGrounded = true;
		return;
	}

	// Tick the re-path cooldown
	if (ai.repathCooldown > 0.0f)
		ai.repathCooldown -= deltaTime;

	// --- Flip detection ---
	// Check if the vehicle's local up vector is pointing downward (flipped over)
	glm::vec3 vehicleUp = transform.quatRotation * glm::vec3(0.0f, 1.0f, 0.0f);
	if (vehicleUp.y < 0.1f) // up vector pointing sideways or downward
	{
		//ai.flippedTimer += deltaTime;
		//if (ai.flippedTimer > ai.flippedTimeThreshold)
		//{
		//	std::cout << "[AI] Flipped over, resetting vehicle" << std::endl;
		//	ai.flippedTimer = 0.0f;

		//	Event resetEvent(Events::Player::RESET_VEHICLE);
		//	resetEvent.SetParam<Entity>(Events::Player::Reset_Vehicle::ENTITY, entity);
		//	controller.SendEvent(resetEvent);

		//	RecomputeNavPath(entity);
		//	TransitionToState(entity, AiState::FollowPath, deltaTime);
		//	return;
		//}
		TransitionToState(entity, AiState::IsFlipped);
		return;
	}
	else
	{
		ai.flippedTimer = 0.0f;
	}

	// Clear passing flag the moment we exit the danger zone
	if (ai.passingThroughDangerZone)
	{
		if (!AiSystemHelperFunctions::IsPointInDangerZone(transform.position))
			{
				ai.passingThroughDangerZone = false;
				ai.dangerDetectionCooldown = ai.dangerDetectionCooldownDuration;
				if (AiSystemHelperFunctions::ShouldLog(m_followPathLogTimer, 0.1f, deltaTime)) {
				std::cout << "[AI] Exited danger zone, starting cooldown ("
					<< ai.dangerDetectionCooldownDuration << "s)" << std::endl;
			}
		}
	}
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
		return;
	}

	/////////////////////////////////
	// --- Special zone checks --- //
	/////////////////////////////////

	// Check waypoints ahead for special zones (similar to danger zone detection)
	uint32_t lookAheadForZones = ai.currentWaypointIndex + 2u; // Look 2 waypoints ahead

	for (uint32_t i = ai.currentWaypointIndex; i < lookAheadForZones && i < static_cast<uint32_t>(ai.navWaypoints.size()); ++i)
	{
		const glm::vec3& waypoint = ai.navWaypoints[i];

		// Boxing Glove Zone Check
		if (AiSystemHelperFunctions::IsInBoxingGloveZone(waypoint)) {
			if (AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 0.5f, deltaTime)) {
				std::cout << "[AI] Boxing glove zone detected ahead, transitioning" << std::endl;
			}
			TransitionToState(entity, AiState::BoxingGloveZone);
			return;
		}

		// Gap zone check
		if (AiSystemHelperFunctions::IsInGapZone(waypoint)) {
			TransitionToState(entity, AiState::GapZone);
			return;
		}

		// Tunnel zone check
		if (AiSystemHelperFunctions::IsInTunnelZone(waypoint)) {
			TransitionToState(entity, AiState::TunnelZone);
			return;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// --- Danger zone check: stop if upcoming waypoints enter a danger zone --- //
	///////////////////////////////////////////////////////////////////////////////

	// Tick danger detection cooldown
	if (ai.dangerDetectionCooldown > 0.0f)
		ai.dangerDetectionCooldown -= deltaTime;

	if (!ai.passingThroughDangerZone && ai.dangerDetectionCooldown <= 0.0f)
	{
		bool pathEntersDanger = false;

		uint32_t lookAheadForDanger = ai.currentWaypointIndex + 3u;

		for (uint32_t i = ai.currentWaypointIndex; i < lookAheadForDanger && i < static_cast<uint32_t>(ai.navWaypoints.size()); ++i)
		{
			if (AiSystemHelperFunctions::IsPointInDangerZone(ai.navWaypoints[i]))
			{
				pathEntersDanger = true;
				break;
			}
		}

		if (pathEntersDanger)
		{
			if (AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 0.5f, deltaTime)) {
				std::cout << "[AI] Danger ahead, entering waiting state" << std::endl;
			}
			TransitionToState(entity, AiState::WaitingAtDangerZone);
			return;
		}
	}

	////////////////////////////////////////////////////////
	// --- Obstacle detection: scan for Bananas ahead --- //
	////////////////////////////////////////////////////////
	{
		glm::vec3 carPos = transform.position;
		glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
		forward.y = 0.0f;
		if (glm::length(forward) > 1e-6f) forward = glm::normalize(forward);

		Entity closestObstacle = 0;
		float closestDist = ai.obstacleDetectionRange;

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
			}
		}

		if (closestObstacle != 0)
		{
			ai.detectedObstacleEntity = closestObstacle;
			ai.avoidTimer = 0.0f;

			std::cout << "[AI] Banana detected at distance " << closestDist << ", avoiding..." << std::endl;

			TransitionToState(entity, AiState::AvoidObstacle);
			return;
		}
	}

	///////////////////////////////////////////////////////////////////
	// Check for nearby powerups (only if we don't already have one) //
	///////////////////////////////////////////////////////////////////
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
			//std::cout << "[AI] Spotted powerup, detouring to collect" << std::endl;
			TransitionToState(entity, AiState::SeekPowerup);
			return;
		}
	}

	// Try to use a held powerup (checked inline -- does NOT interrupt driving)
	if (ai.hasPowerup)
	{
		AiSystemHelperFunctions::TryUsePowerup(entity, gameInstance);
	}

	//////////////////////////////////
	// --- Waypoint advancement --- //
	// ///////////////////////////////

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

	////////////////////////////////
	// --- Progress detection --- //
	////////////////////////////////
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

	///////////////////////////////////////////////////////
	// --- Steering: look-ahead point along the path --- //
	///////////////////////////////////////////////////////
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
		steer = AiSystemHelperFunctions::CalculateSteeringAngle(forward, toSteerXZ);
	}

	/////////////////////////////
	// --- Throttle output --- //
	/////////////////////////////
	float throttle = 0.0f;

	if (speed < ai.desiredSpeed)
	{
		throttle = glm::clamp((ai.desiredSpeed - speed) * ai.throttleKp, 0.0f, ai.maxThrottle);
	}

	//if (ShouldLog(m_followPathLogTimer, 0.1f, deltaTime))
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

	/////////////////////////////
	// --- Detect if stuck --- //
	/////////////////////////////

	// Use desiredSpeed > 0 as intent, not throttle output (which gets scaled down)
	bool wantsToMove = (ai.currentWaypointIndex < static_cast<uint32_t>(ai.navWaypoints.size()));
	if (speed < ai.stuckSpeedThreshold && wantsToMove)
	{
		if (AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 0.5f, deltaTime)) {
			std::cout << "[AI] Potentially stuck: speed=" << speed << " < threshold=" << ai.stuckSpeedThreshold << std::endl;
		}
		TransitionToState(entity, AiState::IsStuck);
		return;
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

	if (AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 0.5f, deltaTime)) {
		std::cout << "[AI] Backing up... timer=" << ai.backupTimer << "s" << std::endl;
	}

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

	//std::cout << "[AI] Backing up... steer=" << steer << std::endl;

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

	float steer = AiSystemHelperFunctions::CalculateSteeringAngle(forward, toWpXZ);

	vc.steer = steer;
	vc.throttle = ai.maxThrottle * 0.5f;
	vc.brake = 0.0f;
	vc.isGrounded = true;
}

//void AiSystem::TryUsePowerup(Entity entity)
//{
//	auto& ai = controller.GetComponent<AiDriver>(entity);
//	auto& vc = controller.GetComponent<VehicleCommands>(entity);
//	if (!ai.hasPowerup || ai.targetPowerupEntity == 0)
//		return;
//	// For simplicity, just use the powerup if we have one and are in the SeekPowerup state
//	if (ai.currentState == AiState::SeekPowerup)
//	{
//		std::cout << "[AI] Using powerup on target " << ai.targetPowerupEntity << std::endl;
//		Event useEvent(Events::Player::USE_POWERUP);
//		useEvent.SetParam<Entity>(Events::Player::Use_Powerup::ENTITY, entity);
//		controller.SendEvent(useEvent);
//		ai.hasPowerup = false;
//		ai.targetPowerupEntity = 0;
//		TransitionToState(entity, AiState::FollowPath, 0.0f);
//	}
//}

void AiSystem::UpdateIsStuckState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);
	ai.stuckTimer += deltaTime;
	if (ai.stuckTimer > ai.stuckTimeThreshold)
	{
		//std::cout << "[AI] STUCK - backing up" << std::endl;
		TransitionToState(entity, AiState::BackingUp);
		return;
	}
}

void AiSystem::UpdateAvoidObstacleState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& body = controller.GetComponent<VehicleBody>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));
	float steer = 0.0f;
	float throttle = 0.0f;
	float brake = 0.0f;

	ai.avoidTimer += deltaTime;

	if (ai.avoidTimer < deltaTime * 1.5f)
	{
		//std::cout << "[AI] Avoiding obstacle..." << std::endl;
		// Calculate distance to obstacle
		float distToObstacle = 999.0f;
		bool obstacleStillThreat = false;


		// 1. Calculate avoidance options :
		glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
		forward.y = 0.0f;
		if (glm::length(forward) > 1e-6f) forward = glm::normalize(forward);

		auto& obsTransform = controller.GetComponent<Transform>(ai.detectedObstacleEntity);
		glm::vec3 toObs = obsTransform.position - transform.position;
		toObs.y = 0.0f;
		distToObstacle = glm::length(toObs);

		//std::cout << "[AI] Checking obstacle: dist=" << distToObstacle << std::endl;

		// Get perpendicular vector to your forward direction
		// Vectors perpendicular to the AI's forward vector
		glm::vec3 right = glm::vec3(forward.z, 0.0f, -forward.x);  // perpendicular right
		glm::vec3 left = glm::vec3(-forward.z, 0.0f, forward.x);   // perpendicular left

		// Banana position
		glm::vec3 bananaPos = obsTransform.position;

		// Generate two candidate positions : leftOffset and rightOffset(e.g., 5 - 8 units to each side of the banana)
		// Generate candidate detour positions
		//These are potential "detour waypoints"
		glm::vec3 leftOffset = bananaPos + (left * ai.avoidOffsetDistance);
		glm::vec3 rightOffset = bananaPos + (right * ai.avoidOffsetDistance);

		// 2.	Query NavMesh for drivability:
		// Use navMesh.FindTriangle(leftOffset) and navMesh.FindTriangle(rightOffset)
		int veerLeft = navMesh.FindTriangle(leftOffset);
		int veerRight = navMesh.FindTriangle(rightOffset);

		bool hasValidPath = false;

		if (veerLeft >= 0 && veerRight >= 0)
		{
			// Both sides are drivable - choose the side closer to the current waypoint (less deviation)
			if (ai.currentWaypointIndex < ai.navWaypoints.size())
			{
				glm::vec3 currentWaypoint = ai.navWaypoints[ai.currentWaypointIndex];

				float distToLeftOffset = glm::length(currentWaypoint - leftOffset);
				float distToRightOffset = glm::length(currentWaypoint - rightOffset);

				if (distToLeftOffset < distToRightOffset)
				{
					ai.temporaryAvoidTarget = leftOffset;
					//std::cout << "[AI] Both sides valid, choosing LEFT (closer to waypoint)" << std::endl;
				}
				else
				{
					ai.temporaryAvoidTarget = rightOffset;
					//std::cout << "[AI] Both sides valid, choosing RIGHT (closer to waypoint)" << std::endl;
				}
			}
			else
			{
				// Fallback: just go left
				ai.temporaryAvoidTarget = leftOffset;
			}
			hasValidPath = true;
		}
		else if (veerLeft >= 0)
		{
			// Only left is drivable
			ai.temporaryAvoidTarget = leftOffset;
			hasValidPath = true;
			//std::cout << "[AI] Choosing LEFT (only valid option)" << std::endl;
		}
		else if (veerRight >= 0)
		{
			// Only right is drivable
			ai.temporaryAvoidTarget = rightOffset;
			hasValidPath = true;
			//std::cout << "[AI] Choosing RIGHT (only valid option)" << std::endl;
		}
		else
		{
			// Neither side is directly drivable - find closest drivable surface
			int32_t closestLeft = navMesh.FindClosestTriangle(leftOffset);
			int32_t closestRight = navMesh.FindClosestTriangle(rightOffset);

			// Choose whichever is closer to valid navmesh
			if (closestLeft >= 0 && closestRight >= 0)
			{
				glm::vec3 leftSurface = navMesh.GetTriangles()[closestLeft].centroid;
				glm::vec3 rightSurface = navMesh.GetTriangles()[closestRight].centroid;

				float leftDist = glm::length(leftOffset - leftSurface);
				float rightDist = glm::length(rightOffset - rightSurface);

				ai.temporaryAvoidTarget = (leftDist < rightDist) ? leftOffset : rightOffset;
				//std::cout << "[AI] Neither side perfect, choosing less bad option" << std::endl;
			}
			else
			{
				// Last resort: just pick left and hope for the best
				ai.temporaryAvoidTarget = leftOffset;
				//std::cout << "[AI] WARNING: No good avoidance path found, trying left anyway" << std::endl;
			}
			hasValidPath = false;  // mark as risky
		}


		// 4. Steering behavior :
		// Get vector from car to the temporary avoidance target

		// Calculate steering angle using your existing function

		// Get vector from car to the temporary avoidance target
		glm::vec3 toAvoidTarget = ai.temporaryAvoidTarget - transform.position;
		toAvoidTarget.y = 0.0f;  // flatten to horizontal plane

		// Calculate steering angle using your existing function
		steer = AiSystemHelperFunctions::CalculateSteeringAngle(forward, toAvoidTarget);

		// Reduce speed for safer maneuvering
		//float targetAvoidSpeed = ai.desiredSpeed * 0.75f;  // 75% of normal speed

		//if (speed < targetAvoidSpeed)
		//{
		//	std::cout << "[AI] Slowing down for avoidance, current speed: " << speed << std::endl;
		//	throttle = glm::clamp((targetAvoidSpeed - speed) * ai.throttleKp, 0.0f, ai.maxThrottle * 0.8f);
		//}
		//else
		//{
		//	throttle = 0.0f;  // coast if going too fast
		//	brake = 0.3f;     // light braking to slow down
		//}

		// 5. Exit condition :
		// Use dot product : dot(forward, toBanana) < 0 means banana is now behind you
		// Check dot product: is banana behind us now?
		glm::vec3 toBanana = bananaPos - transform.position;
		toBanana.y = 0.0f;

		float dotToBanana = glm::dot(forward, glm::normalize(toBanana));

		//if (dotToBanana < 0.0f)  // banana is now behind us
		//{
		//	std::cout << "test2" << std::endl;
		//	std::cout << "[AI] Banana passed, resuming normal path" << std::endl;
		//}

		ai.temporaryAvoidTarget = glm::vec3(0.0f);  // clear the avoidance target
		ai.detectedObstacleEntity = 0;
		RecomputeNavPath(entity);  // get back on track
		TransitionToState(entity, AiState::FollowPath);
		return;

		// Alternative exit condition: if we have a valid path and are close to the temporary target, exit avoidance
		// float distToAvoidTarget = glm::length(toAvoidTarget);

		//if (distToAvoidTarget < ai.arrivalRadius)  // arrived at detour point
		//{
		//	std::cout << "[AI] Reached avoidance waypoint, resuming path" << std::endl;
		//	// same cleanup as above
		//}
		//	•	Clear temporaryAvoidTarget and isAvoidingBanana flag
		//	•	Resume normal FollowPath behavior
	}

	vc.steer = steer;
	vc.throttle = throttle;
	vc.brake = brake;
	vc.isGrounded = true;
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

	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));

	// 1) Get distance to the danger zone
	float distToZone = AiSystemHelperFunctions::GetDistanceToDangerZone(carPos);

	// 2) Check if obstacle is blocking
	bool isBlocked = AiSystemHelperFunctions::IsArmBlocking(carPos);

	float SLOW_DISTANCE = 30.0f;
	float STOP_DISTANCE = 10.0f;

	// 3) Calculate steering toward next waypoint
	// EXCEPT in boxing glove zone - always drive straight there
	float steer = 0.0f;
	if (!AiSystemHelperFunctions::IsInBoxingGloveZone(carPos) && ai.currentWaypointIndex < ai.navWaypoints.size())
	{
		glm::vec3 toWp = ai.navWaypoints[ai.currentWaypointIndex] - carPos;
		toWp.y = 0.0f;
		steer = AiSystemHelperFunctions::CalculateSteeringAngle(forward, toWp);
	}

	// 4) Speed control based on distance
	if (distToZone > SLOW_DISTANCE)
	{
		// Far away - normal driving
		vc.throttle = 1.0f;
		vc.brake = 0.0f;
	}
	else if (distToZone > STOP_DISTANCE)
	{
		// Approaching - slow down
		vc.throttle = 0.0f;
		vc.brake = 0.5f;
	}
	else
	{
		// AT THE BOUNDARY
		if (isBlocked)
		{
			// STOP and HOLD (brake=0 to avoid reversing!)
			vc.throttle = 0.0f;
			vc.brake = 0.0f;  // ← Keep this at 0 to prevent backing up
			steer = 0.0f;     // ← Hold straight when stopped
		}
		else
		{
			// Clear to go!
			if (AiSystemHelperFunctions::ShouldLog(m_waitingLogTimer, 1.0f, deltaTime)) {
				std::cout << "[AI] Danger zone clear, resuming path" << std::endl;
			}
			vc.throttle = 1.0f;
			vc.brake = 0.0f;

			ai.passingThroughDangerZone = true;
			TransitionToState(entity, AiState::FollowPath);
			return;
		}
	}

	vc.steer = steer;
	vc.isGrounded = true;
}

void AiSystem::UpdateBoxingGloveZoneState(Entity entity, float deltaTime)
{
	auto& vc = controller.GetComponent<VehicleCommands>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	if(AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 1.0f, deltaTime)) {
		std::cout << "Entering Boxing Glove zone" << std::endl;
	}

	// MANUAL CONTROL: Full throttle, steer straight
	vc.throttle = 1.0f;
	vc.brake = 0.0f;
	vc.steer = 0.0f;
	vc.isGrounded = true;

	if(AiSystemHelperFunctions::GetDistanceToDangerZone(transform.position) < 10.0f)
	{
		TransitionToState(entity, AiState::WaitingAtDangerZone);
		return;
	}

	// Exit check: am I past the gap?
	if (transform.position.x < -76.0f) { // afterGap X coordinate
		TransitionToState(entity, AiState::FollowPath);
	}
}

void AiSystem::UpdateGapZoneState(Entity entity, float deltaTime)
{
	auto& vc = controller.GetComponent<VehicleCommands>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	// MANUAL CONTROL: Full throttle, steer straight
	vc.throttle = 1.0f;
	vc.brake = 0.0f;
	vc.steer = 0.0f;
	vc.isGrounded = true;

	// Exit check: am I past the gap?
	if (transform.position.x > 155.0f) { // afterGap X coordinate
		TransitionToState(entity, AiState::FollowPath);
	}
}

void AiSystem::UpdateTunnelZoneState(Entity entity, float deltaTime) {
	auto& vc = controller.GetComponent<VehicleCommands>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	// MANUAL CONTROL: Full throttle, steer straight
	vc.throttle = 1.0f;
	vc.brake = 0.0f;
	vc.steer = 0.0f;
	vc.isGrounded = true;

	// Exit check: am I past the gap?
	if (transform.position.x > 150.0f) { // afterGap X coordinate
		TransitionToState(entity, AiState::FollowPath);
	}
}

void AiSystem::UpdateFloorItState(Entity entity, float deltaTime)
{
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

		if (gameInstance)
			gameInstance->SchedulePowerupRespawn(ai.targetPowerupEntity);

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

	float steer = AiSystemHelperFunctions::CalculateSteeringAngle(forward, toPowerupXZ);

	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));
	float throttle = glm::clamp((ai.desiredSpeed * 0.7f - speed) * ai.throttleKp, 0.0f, ai.maxThrottle);

	vc.steer = steer;
	vc.throttle = throttle;
	vc.brake = 0.0f;
	vc.isGrounded = true;
}

void AiSystem::UpdateIsFlippedState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& body = controller.GetComponent<VehicleBody>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

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

float AiSystem::CalculateDistanceToFinish(const glm::vec3& position) const
{
	if (entities.empty())
		return 0.0f;

	Entity aiEntity = *entities.begin();
	if (!controller.HasComponent<AiDriver>(aiEntity))
		return 0.0f;

	const auto& navWaypoints = controller.GetComponent<AiDriver>(aiEntity).navWaypoints;

	if (navWaypoints.empty())
		return 0.0f;

	int closestIndex = 0;
	float closestDist = (std::numeric_limits<float>::max)();
	for (int i = 0; i < static_cast<int>(navWaypoints.size()); i++)
	{
		float d = glm::length(navWaypoints[i] - position);
		if (d < closestDist)
		{
			closestDist = d;
			closestIndex = i;
		}
	}

	float total = 0.0f;

	if (closestIndex < static_cast<int>(navWaypoints.size()) - 1)
	{
		glm::vec3 segDir = glm::normalize(navWaypoints[closestIndex + 1] - navWaypoints[closestIndex]);
		glm::vec3 toPlayer = position - navWaypoints[closestIndex];
		float projectedAlong = glm::dot(toPlayer, segDir);

		total = glm::length(navWaypoints[closestIndex + 1] - navWaypoints[closestIndex]) - projectedAlong;
		closestIndex++;
	}

	for (int i = closestIndex; i < static_cast<int>(navWaypoints.size()) - 1; i++)
	{
		total += glm::length(navWaypoints[i + 1] - navWaypoints[i]);
	}

	return total > 0.0f ? total : 0.0f;
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

// ===== PUBLIC WRAPPER FUNCTIONS =====
// These delegate to AiSystemHelperFunctions for backward compatibility

float AiSystem::CheckDangerZone(const glm::vec3& position) const
{
	return AiSystemHelperFunctions::CheckDangerZone(position);
}

bool AiSystem::IsInGapZone(const glm::vec3& pos) const
{
	return AiSystemHelperFunctions::IsInGapZone(pos);
}

bool AiSystem::IsInTunnelZone(const glm::vec3& pos) const
{
	return AiSystemHelperFunctions::IsInTunnelZone(pos);
}

bool AiSystem::IsInBoxingGloveZone(const glm::vec3& pos) const
{
	return AiSystemHelperFunctions::IsInBoxingGloveZone(pos);
}