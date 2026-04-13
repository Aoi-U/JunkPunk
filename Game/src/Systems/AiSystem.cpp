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
#include <algorithm>

extern ECSController controller;

// Helper function to convert AiState enum to string
static const char* AiStateToString(AiState state)
{
	switch (state)
	{
	case AiState::FollowPath: return "FollowPath";
	case AiState::BackingUp: return "BackingUp";
	case AiState::IsStuck: return "IsStuck";
	case AiState::IsFlipped: return "IsFlipped";
	case AiState::RecoveringFromOffTrack: return "RecoveringFromOffTrack";
	case AiState::WaitingAtDangerZone: return "WaitingAtDangerZone";
	case AiState::BoxingGloveZone: return "BoxingGloveZone";
	case AiState::GapZone: return "GapZone";
	case AiState::TunnelZone: return "TunnelZone";
	case AiState::AvoidObstacle: return "AvoidObstacle";
	case AiState::SeekPowerup: return "SeekPowerup";
	case AiState::UsePowerup: return "UsePowerup";
	case AiState::Overtaking: return "Overtaking";
	case AiState::Braking: return "Braking";
	case AiState::FloorIt: return "FloorIt";
	default: return "Unknown";
	}
}

void AiSystem::Init()
{
	// { glm::vec3(point to pathfind to), bottom of segement, top of segment }
	courseSegments = {
		// Segment 0: Start area / bottom level
		{ glm::vec3(55.0f, -178.0f, -58.0f),   -260.0f, -178.0f, true  },  // beforeBoxingGloves

		// Segment 2: Gap area
		{ glm::vec3(-60.0f, -31.0f, 170.0f),    -178.0f,  -29.0f, false },  // beforeGap

		// Segment 3: After gap, pre-tunnel
		{ glm::vec3(150.0f, -31.0f, 244.0f),    -29.0f,  54.011f, false },  // beforeTunnel

		// Segment 4: Mid tunnel
		{ glm::vec3(-135.6f, -28.0f, 392.5f),   -29.0f,  54.011f, false },  // midTunnel

		// Segment 5: Inside tunnel
		{ glm::vec3(-62.0f, -26.0f, 314.0f),    -29.0f,  54.011f, false },  // inTunnel

		// Segment 6: Upper track
		{ glm::vec3(107.854f, 54.011f, 318.24f),  54.011f,  100.0f, false },  // uppertrack

		// Segment 7: Goal area
		{ glm::vec3(-70.0f, 56.0f, 326.0f),      54.011f,  100.0f, false },  // goal
	};
}

void AiSystem::SetSpinnerInfos(const std::vector<SpinnerInfo>& infos)
{
	spinnerInfos = infos;
}

void AiSystem::SetNavMesh(const NavMesh& mesh)
{
	navMesh = mesh;
}

void AiSystem::ComputeNavPath(Entity entity)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	glm::vec3 startPos = transform.position;
	glm::vec3 beforeBoxingGloves(55.0f, -178.0f, -58.0f);
	glm::vec3 afterBoxingGloves(-71.0f, -178.0f, -58.0f);
	glm::vec3 beforeGap(-60.0f, -31.0f, 170.0f);
	glm::vec3 afterGap(150.0f, -28.0f, 185.0f);
	glm::vec3 beforeTunnel(150.0f, -31.0f, 244.0f);
	glm::vec3 midTunnel(-135.600f, -28.000f, 392.500f);
	glm::vec3 inTunnel(-62.000f, -26.000f, 314.000f);
	glm::vec3 uppertrack(107.854f, 54.011f, 318.240f);

	// SEGMENT 1: Start -> beforeBoxingGloves
	int32_t startTri = navMesh.FindTriangle(startPos);
	if (startTri < 0) startTri = navMesh.FindClosestTriangle(startPos);
	int32_t beforeBoxingGlovesTri = navMesh.FindTriangle(beforeBoxingGloves);
	if (beforeBoxingGlovesTri < 0) beforeBoxingGlovesTri = navMesh.FindClosestTriangle(beforeBoxingGloves);
	NavPath path1 = navMesh.FindPath(startTri, beforeBoxingGlovesTri, startPos, beforeBoxingGloves);

	// SEGMENT 2: afterBoxingGloves -> beforeGap
	int32_t afterBoxingGlovesTri = navMesh.FindTriangle(afterBoxingGloves);
	if (afterBoxingGlovesTri < 0) afterBoxingGlovesTri = navMesh.FindClosestTriangle(afterBoxingGloves);
	int32_t beforeGapTri = navMesh.FindTriangle(beforeGap);
	if (beforeGapTri < 0) beforeGapTri = navMesh.FindClosestTriangle(beforeGap);
	NavPath path2 = navMesh.FindPath(afterBoxingGlovesTri, beforeGapTri, afterBoxingGloves, beforeGap);

	// SEGMENT 3: After gap -> before tunnel
	// Use FindTriangleAtHeight to avoid matching ceiling/wall triangles
	int32_t afterGapTri = navMesh.FindTriangleAtHeight(afterGap, 5.0f);
	if (afterGapTri < 0) afterGapTri = navMesh.FindClosestTriangleAtHeight(afterGap, 5.0f);

	int32_t beforeTunnelTri = navMesh.FindTriangleAtHeight(beforeTunnel, 5.0f);
	if (beforeTunnelTri < 0) beforeTunnelTri = navMesh.FindClosestTriangleAtHeight(beforeTunnel, 5.0f);
	NavPath path3 = navMesh.FindPath(afterGapTri, beforeTunnelTri, afterGap, beforeTunnel);

	// SEGMENT 4: Before tunnel -> mid tunnel
	int32_t midTunnelTri = navMesh.FindTriangleAtHeight(midTunnel, 5.0f);
	if (midTunnelTri < 0) midTunnelTri = navMesh.FindClosestTriangleAtHeight(midTunnel, 5.0f);
	NavPath path4 = navMesh.FindPath(beforeTunnelTri, midTunnelTri, beforeTunnel, midTunnel);

	// SEGMENT 5: Mid tunnel -> inside tunnel
	int32_t inTunnelTri = navMesh.FindTriangle(inTunnel);
	if (inTunnelTri < 0) inTunnelTri = navMesh.FindClosestTriangle(inTunnel);
	NavPath path5 = navMesh.FindPath(midTunnelTri, inTunnelTri, midTunnel, inTunnel);

	// SEGMENT 6: Inside tunnel -> upper track
	int32_t upperTrackTri = navMesh.FindTriangle(uppertrack);
	if (upperTrackTri < 0) upperTrackTri = navMesh.FindClosestTriangle(uppertrack);
	NavPath path6 = navMesh.FindPath(inTunnelTri, upperTrackTri, inTunnel, uppertrack);

	// SEGMENT 7: Upper track -> goal
	int32_t goalTri = navMesh.FindTriangle(goalPosition);
	if (goalTri < 0) goalTri = navMesh.FindClosestTriangle(goalPosition);
	NavPath path7 = navMesh.FindPath(upperTrackTri, goalTri, uppertrack, goalPosition);

	// Assemble the full waypoint path from all segments
	ai.navWaypoints = path1.waypoints;

	// Manual boxing glove crossing
	ai.navWaypoints.push_back(beforeBoxingGloves);
	ai.navWaypoints.push_back(afterBoxingGloves);

	// Segment 2 (afterBoxingGloves -> beforeGap)
	ai.navWaypoints.insert(ai.navWaypoints.end(),
		path2.waypoints.begin(),
		path2.waypoints.end());

	// Manual gap crossing
	ai.navWaypoints.push_back(beforeGap);
	ai.navWaypoints.push_back(afterGap);

	// Segment 3 (afterGap -> beforeTunnel)
	if (!path3.waypoints.empty())
	{
		ai.navWaypoints.insert(ai.navWaypoints.end(),
			path3.waypoints.begin(),
			path3.waypoints.end());
	}

	ai.navWaypoints.push_back(beforeTunnel);

	// Segment 4 (beforeTunnel -> midTunnel)
	if (!path4.waypoints.empty())
	{
		ai.navWaypoints.insert(ai.navWaypoints.end(),
			path4.waypoints.begin(),
			path4.waypoints.end());
	}
	else
	{
		ai.navWaypoints.push_back(midTunnel);  // Only if A* failed
	}

	// Segment 5 (midTunnel -> inTunnel)
	if (!path5.waypoints.empty())
	{
		ai.navWaypoints.insert(ai.navWaypoints.end(),
			path5.waypoints.begin(),
			path5.waypoints.end());
	}
	else
	{
		ai.navWaypoints.push_back(inTunnel);  // Only if A* failed
	}

	// Segment 6 (inTunnel -> uppertrack)
	if (!path6.waypoints.empty())
	{
		ai.navWaypoints.insert(ai.navWaypoints.end(),
			path6.waypoints.begin(),
			path6.waypoints.end());
	}
	else
	{
		ai.navWaypoints.push_back(uppertrack);  // Only if A* failed
	}

	// Bridge to Segment 7
	ai.navWaypoints.push_back(uppertrack);

	// Segment 7 (uppertrack -> goal)
	ai.navWaypoints.insert(ai.navWaypoints.end(),
		path7.waypoints.begin(),
		path7.waypoints.end());

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
	ai.stuckTimer = 0.0f;
	ai.isBypassingSpinner = false;
	ai.shouldJumpSpinner = false;
	ai.hasJumpedCurrentSpinner = false;

	//std::cout << "[AiSystem] Re-pathed: " << path.triangleIndices.size()
	//	<< " corridor tris -> " << ai.navWaypoints.size()
	//	<< " waypoints, starting at wp[" << ai.currentWaypointIndex << "]" << std::endl;
}

int AiSystem::FindCurrentSegment(const glm::vec3& pos) const {
	if (courseSegments.empty()) return 0;

	int lastIndex = static_cast<int>(courseSegments.size()) - 1;

	// Height checks for unambiguous levels
	if (pos.y < -178.0f) return 0;   // Bottom level
	if (pos.y > 45.0f)   return lastIndex;  // Upper level

	// Mid level: find closest entry point using XZ distance
	int best = 1;
	float bestDist = FLT_MAX;
	for (int i = 1; i < static_cast<int>(courseSegments.size()); i++) {
		glm::vec3 diff = pos - courseSegments[i].entryPoint;
		diff.y = 0.0f;
		float d = glm::length(diff);
		if (d < bestDist) {
			bestDist = d;
			best = i;
		}
	}
	return best;
}

void AiSystem::Update(float deltaTime)
{
	for (auto entity : entities)
	{
		auto& ai = controller.GetComponent<AiDriver>(entity);
		auto& transform = controller.GetComponent<Transform>(entity);
		auto& body = controller.GetComponent<VehicleBody>(entity);
		auto& vc = controller.GetComponent<VehicleCommands>(entity);

		// Calculate speed ONCE per frame
		ai.currentSpeed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));

		// Deferred repath: vehicle was teleported last frame, Transform is now up to date
		if (ai.needsRepath)
		{
			ai.needsRepath = false;
			int seg = FindCurrentSegment(transform.position);
			if (courseSegments[seg].resetBoxingGloves)
			{
				ai.currentBoxingGloveIndex = 0;
				ComputeNavPath(entity);
			}
			else
			{
				RecomputeNavPath(entity);
			}
			std::cout << "[AI] Deferred repath complete (segment " << seg << ")" << std::endl;
		}

		//if (AiSystemHelperFunctions::ShouldLog(m_followPathLogTimer, 1.0f, deltaTime))
		//{
		//	std::cout << "[AiSystem::Update] Entity=" << entity
		//		<< " | State=" << static_cast<int>(ai.currentState)
		//		<< " | Waypoints=" << ai.navWaypoints.size()
		//		<< " | CurrentWP=" << ai.currentWaypointIndex
		//		<< " | Pos=(" << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << ")"
		//		<< std::endl;
		//}

		if (vc.isGrounded)
		{
			glm::vec3 vehicleUp = transform.quatRotation * glm::vec3(0.0f, 1.0f, 0.0f);
			if (vehicleUp.y > 0.5f)  // only record upright positions
				ai.lastGroundedPosition = transform.position;
		}

		if (ai.navWaypoints.empty() && !navMesh.IsEmpty())
		{
			int seg = FindCurrentSegment(transform.position);
			if (seg > 0)
				RecomputeNavPath(entity);   // Mid-track spawn: simple A* to goal
			else
				ComputeNavPath(entity);     // Normal start: full segmented path
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

void AiSystem::TransitionToState(Entity entity, AiState newState, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);

	// Early exit if already in this state
	if (ai.currentState == newState)
	{
		if (AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 1.0f, deltaTime)) {
			std::cout << "[AI] Already in state " << AiStateToString(newState) << std::endl;
		}
		return;
	}
	if (AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 1.0f, deltaTime)) {
		std::cout << "[AI] Transitioning from state " << AiStateToString(ai.currentState)
			<< " to " << AiStateToString(newState) << std::endl;
	}

	ai.previousState = ai.currentState;
	ai.currentState = newState;

	// Reset timers when entering specific states
	switch (newState)
	{
	case AiState::FollowPath:
		ai.stuckTimer = 0.0f;
		ai.shouldJumpSpinner = false;
		ai.hasJumpedCurrentSpinner = false;
		ai.isBypassingSpinner = false;
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

	// Calculate forward vector once (used throughout function)
	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	glm::vec3 spinnerBypassTarget = glm::vec3(0.0f);
	bool hasSpinnerOverride = false;

	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));

	// Dynamic lookahead: the faster we go, the further ahead we steer
	float lookahead = ai.lookaheadDistance + speed * 0.3f;

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

	////////////////////////////
	// --- Flip detection --- //
	// /////////////////////////
	
	// Check if the vehicle's local up vector is pointing downward (flipped over)
	glm::vec3 vehicleUp = transform.quatRotation * glm::vec3(0.0f, 1.0f, 0.0f);
	if (vehicleUp.y < 0.1f && vc.isGrounded) // up vector pointing sideways or downward
	{
		TransitionToState(entity, AiState::IsFlipped, deltaTime);
		return;
	}
	else
	{
		ai.flippedTimer = 0.0f;
	}

	/////////////////////////
	// Off-track detection //
	// //////////////////////
	
	// check if car is far below the nearest navmesh surface
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
		TransitionToState(entity, AiState::RecoveringFromOffTrack, deltaTime);
		return;
	}

	/////////////////////////////////
	// --- Special zone checks --- //
	/////////////////////////////////

	// Use cone-based detection for special zones (reuse carPos and forward from above)
	// Use lookahead distance for zone detection
	float zoneDetectionRange = ai.lookaheadDistance * 3.0f; // ~45 units ahead
	float zoneDetectionCone = 0.1f;

	// Check if we're approaching the beforeBoxingGloves waypoint
	glm::vec3 beforeBoxingGloves(50.0f, -178.0f, -58.0f);

	if (ai.currentWaypointIndex < ai.navWaypoints.size())
	{
		glm::vec3 currentWp = ai.navWaypoints[ai.currentWaypointIndex];
		float distToBeforeBoxingGloves = glm::length(currentWp - beforeBoxingGloves);
		if (ai.currentBoxingGloveIndex < boxingGloveDangerZones.size()) {
			if (distToBeforeBoxingGloves < 10.0f) // Slow down zone
			{
				if (ai.currentSpeed < 0.5f || distToBeforeBoxingGloves < 3.0f) // Nearly stopped
				{
					std::cout << "[AI] Transitioning to BoxingGloveZone state" << std::endl;
					// Transition to boxing glove zone state
					TransitionToState(entity, AiState::BoxingGloveZone, deltaTime);
					return;
				}

				// Apply braking
				vc.throttle = 0.0f;
				vc.brake = 0.2f;
				vc.steer = 0.0f;
				vc.isGrounded = true;
				return;
			}
		}
	}

	// Gap zone check
	float distToGap = AiSystemHelperFunctions::ScanForGapZoneInCone(
		carPos, forward, zoneDetectionRange, zoneDetectionCone);

	if (distToGap > 0.0f) {
		if (AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 0.5f, deltaTime)) {
			std::cout << "[AI] Gap zone detected at distance " << distToGap << ", transitioning" << std::endl;
		}
		TransitionToState(entity, AiState::GapZone, deltaTime);
		return;
	}

	// Tunnel zone check
	float distToTunnel = AiSystemHelperFunctions::ScanForTunnelZoneInCone(
		carPos, forward, zoneDetectionRange, zoneDetectionCone);

	if (distToTunnel > 0.0f) {
		if (AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 0.5f, deltaTime)) {
			std::cout << "[AI] Tunnel zone detected at distance " << distToTunnel << ", transitioning" << std::endl;
		}
		TransitionToState(entity, AiState::TunnelZone, deltaTime);
		return;
	}

	////////////////////////////////////////////////////////
	// --- Obstacle detection: scan for Bananas ahead --- //
	////////////////////////////////////////////////////////
	{
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

			TransitionToState(entity, AiState::AvoidObstacle, deltaTime);
			return;
		}
	}

	/////////////////////////////////////////////////////////
	// --- Obstacle detection: scan for spinners ahead --- //
	/////////////////////////////////////////////////////////
	if (!ai.isBypassingSpinner && !ai.shouldJumpSpinner && ai.spinnerJumpCooldown <= 0.0f)
	{
		float closestSpinnerDist = FLT_MAX;
		glm::vec3 bestBypassTarget = glm::vec3(0.0f);
		bool foundSpinner = false;
		float closestSpinnerRadius = 0.0f;
		glm::vec3 closestSpinnerPos = glm::vec3(0.0f);

		for (const auto& spinner : spinnerInfos)
		{
			glm::vec3 toSpinner = spinner.position - carPos;
			toSpinner.y = 0.0f;
			float dist = glm::length(toSpinner);

			if (dist < 1e-5f || dist > spinner.radius + ai.lookaheadDistance)
				continue;

			float dot = glm::dot(forward, glm::normalize(toSpinner));
			if (dot < 0.3f)
				continue;

			glm::vec3 toSpinnerN = glm::normalize(toSpinner);
			glm::vec3 rightOfApproach = glm::vec3(toSpinnerN.z, 0.0f, -toSpinnerN.x);
			glm::vec3 leftOfApproach = glm::vec3(-toSpinnerN.z, 0.0f, toSpinnerN.x);

			float bypassDist = (std::min)(spinner.radius + 6.0f, 18.0f);

			glm::vec3 bypassTarget;
			if (spinner.isClockwise)
				bypassTarget = spinner.position + leftOfApproach * bypassDist;
			else
				bypassTarget = spinner.position + rightOfApproach * bypassDist;

			int32_t bypassTri = navMesh.FindTriangleAtHeight(bypassTarget, 5.0f);
			if (bypassTri < 0)
			{
				if (spinner.isClockwise)
					bypassTarget = spinner.position + rightOfApproach * bypassDist;
				else
					bypassTarget = spinner.position + leftOfApproach * bypassDist;

				bypassTri = navMesh.FindTriangleAtHeight(bypassTarget, 5.0f);
				if (bypassTri < 0)
					continue;
			}

			if (dist < closestSpinnerDist)
			{
				closestSpinnerDist = dist;
				bestBypassTarget = bypassTarget;
				closestSpinnerRadius = spinner.radius;
				closestSpinnerPos = spinner.position;
				foundSpinner = true;
			}
		}

		if (foundSpinner)
		{
			if (closestSpinnerRadius <= ai.spinnerJumpRadiusThreshold)
			{
				// Small spinner — jump over it
				ai.shouldJumpSpinner = true;
				ai.jumpSpinnerPosition = closestSpinnerPos;
				ai.jumpSpinnerRadius = closestSpinnerRadius;
				ai.hasJumpedCurrentSpinner = false;
			}
			else
			{
				// Large spinner — steer around it (existing behavior)
				ai.isBypassingSpinner = true;
				ai.lockedBypassTarget = bestBypassTarget;
				ai.spinnerBypassTimer = 0.0f;
			}
		}
	}

	if (ai.isBypassingSpinner)
	{
		spinnerBypassTarget = ai.lockedBypassTarget;
		hasSpinnerOverride = true;
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

			// Skip powerups on different vertical levels
			float heightDiff = std::abs(toPowerup.y);
			if (heightDiff > 10.0f)
				continue;

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
			TransitionToState(entity, AiState::SeekPowerup, deltaTime);
			return;
		}
	}

	// Try to use a held powerup
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
	// (forward vector already computed at top of function)

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
			<< "] (" << ai.navWaypoints[ai.currentWaypointIndex].x << ", " << ai.navWaypoints[ai.currentWaypointIndex].y << ", " << ai.navWaypoints[ai.currentWaypointIndex].z << "), re-pathing from current position" << std::endl;
		RecomputeNavPath(entity);
		ai.repathCooldown = ai.repathCooldownDuration;
		return;
	}

	///////////////////////////////////////////////////////
	// --- Steering: look-ahead point along the path --- //
	//////////////////////////////////////////////////////

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

	// Handle spinner jump (small spinners)
	if (ai.shouldJumpSpinner)
	{
		glm::vec3 toSpinner = ai.jumpSpinnerPosition - carPos;
		toSpinner.y = 0.0f;
		float distToSpinner = glm::length(toSpinner);

		if (distToSpinner > 1e-5f)
		{
			float dotToSpinner = glm::dot(forward, glm::normalize(toSpinner));

			// Spinner is behind us — clear the flag
			if (dotToSpinner < 0.0f)
			{
				ai.shouldJumpSpinner = false;
				ai.hasJumpedCurrentSpinner = false;
				ai.spinnerJumpCooldown = ai.spinnerJumpCooldownDuration;
			}
			// Close enough to jump and grounded
			else if (!ai.hasJumpedCurrentSpinner && vc.isGrounded)
			{
				// Dynamic trigger: jump earlier at higher speeds
				float dynamicTrigger = ai.spinnerJumpTriggerDistance + speed * 0.2f;
				if (distToSpinner < dynamicTrigger)
				{
					Event jumpEvent(Events::Player::PLAYER_JUMPED);
					jumpEvent.SetParam<Entity>(Events::Player::Player_Jumped::ENTITY, entity);
					controller.SendEvent(jumpEvent);
					ai.hasJumpedCurrentSpinner = true;
					std::cout << "[AI] Jumping over spinner (radius=" << ai.jumpSpinnerRadius
						<< ", dist=" << distToSpinner << ")" << std::endl;
				}
			}
		}
		else
		{
			ai.shouldJumpSpinner = false;
			ai.hasJumpedCurrentSpinner = false;
		}
	}

	// Override steering if spinner avoidance is active
	if (hasSpinnerOverride)
	{
		ai.spinnerBypassTimer += deltaTime;

		glm::vec3 toBypass = ai.lockedBypassTarget - carPos;
		toBypass.y = 0.0f;
		float distToBypass = glm::length(toBypass);

		if (distToBypass < ai.arrivalRadius)
		{
			ai.isBypassingSpinner = false;
		}
		else if (ai.spinnerBypassTimer > ai.spinnerBypassTimeout)
		{
			ai.isBypassingSpinner = false;
			std::cout << "[AI] Spinner bypass timeout, re-pathing" << std::endl;
			RecomputeNavPath(entity);
			return;
		}
		else
		{
			steer = AiSystemHelperFunctions::CalculateSteeringAngle(forward, toBypass);
		}
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
	bool tryingToMove = (throttle > 0.1f);  // Actually applying throttle

	if (speed < ai.stuckSpeedThreshold && wantsToMove && tryingToMove)
	{
		ai.stuckTimer += deltaTime;

		if (ai.stuckTimer > ai.stuckTimeThreshold)
		{
			if (AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 0.5f, deltaTime)) {
				std::cout << "[AI] Stuck for " << ai.stuckTimer << "s (speed=" << speed
					<< " < threshold=" << ai.stuckSpeedThreshold << ")" << std::endl;
			}
			TransitionToState(entity, AiState::IsStuck, deltaTime);
			return;
		}
	}
	else
	{
		ai.stuckTimer = 0.0f;  // Reset if making progress or not trying to move
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
		TransitionToState(entity, AiState::FollowPath, deltaTime);
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
		TransitionToState(entity, AiState::FollowPath, deltaTime);
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
			TransitionToState(entity, AiState::FollowPath, deltaTime);
			return;
		}
	}

	// Re-path if stuck for too long, but respect cooldown
	if (ai.recoveryTimer > 5.0f && ai.repathCooldown <= 0.0f)
	{
		std::cout << "[AI] Recovery timeout, re-pathing..." << std::endl;
		ai.recoveryTimer = 0.0f;
		int seg = FindCurrentSegment(transform.position);
		if (courseSegments[seg].resetBoxingGloves)
		{
			// Fell back to bottom level — rebuild full segmented path
			ai.currentBoxingGloveIndex = 0;
			ComputeNavPath(entity);
		}
		else
		{
			RecomputeNavPath(entity);
			return;
		}
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
		TransitionToState(entity, AiState::BackingUp, deltaTime);
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
		TransitionToState(entity, AiState::FollowPath, deltaTime);
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

	// Check the SPECIFIC glove we're waiting for (by index)
	bool isBlocked = true;  // default: blocked

	if (ai.currentBoxingGloveIndex < static_cast<uint32_t>(boxingGloveDangerZones.size()))
	{
		Entity targetDzEntity = boxingGloveDangerZones[ai.currentBoxingGloveIndex];
		auto& dz = controller.GetComponent<DangerZone>(targetDzEntity);

		if (dz.obstacleEntity != 0 && controller.HasComponent<MovingObstacle>(dz.obstacleEntity))
		{
			auto& obstacle = controller.GetComponent<MovingObstacle>(dz.obstacleEntity);
			// Blocked if arm is not fully retracted (index 3 = retracted)
			isBlocked = (obstacle.currentPathIndex < 3);
		}
		else
		{
			isBlocked = false;  // No obstacle linked, safe to pass
		}
	}
	else
	{
		isBlocked = false;  // Past all indexed gloves
	}

	float steer = 0.0f;

	if (isBlocked)
	{
		// STOP and HOLD
		vc.throttle = 0.0f;
		vc.brake = 0.0f;
		steer = 0.0f;
	}
	else
	{
		// Clear to go! Advance to the NEXT glove
		if (AiSystemHelperFunctions::ShouldLog(m_waitingLogTimer, 0.1f, deltaTime)) {
			std::cout << "[AI] Glove " << ai.currentBoxingGloveIndex << " clear, advancing to next" << std::endl;
		}
		vc.throttle = 1.0f;
		vc.brake = 0.0f;

		ai.currentBoxingGloveIndex++;  // Target the next glove
		ai.passingThroughDangerZone = true;
		TransitionToState(entity, AiState::BoxingGloveZone, deltaTime);
		return;
	}

	vc.steer = steer;
	vc.isGrounded = true;
}

void AiSystem::UpdateBoxingGloveZoneState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	if(AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 1.0f, deltaTime)) {
		std::cout << "[AI] In Boxing Glove zone, targeting glove " << ai.currentBoxingGloveIndex
			<< " of " << boxingGloveDangerZones.size() << std::endl;
	}

	// Calculate forward direction and steering toward exit point
	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	// Exit check: am I past the entire zone?
	if (transform.position.x < -76.0f) {
		if (AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 0.1f, deltaTime)) {
			std::cout << "[AI] Passed through Boxing Glove zone, resuming path" << std::endl;
		}

		// Advance waypoint index past the boxing glove waypoints (both are behind the car now)
		while (ai.currentWaypointIndex + 1 < static_cast<uint32_t>(ai.navWaypoints.size()))
		{
			glm::vec3 toWp = ai.navWaypoints[ai.currentWaypointIndex] - transform.position;
			toWp.y = 0.0f;
			float dist = glm::length(toWp);
			if (dist > 1e-5f)
			{
				float dot = glm::dot(forward, glm::normalize(toWp));
				if (dot > 0.0f && dist > ai.arrivalRadius)
					break;  // Found a waypoint ahead of us (segment 2+)
			}
			ai.currentWaypointIndex++;
		}

		std::cout << "[AI] Resuming at waypoint " << ai.currentWaypointIndex << std::endl;
		TransitionToState(entity, AiState::FollowPath, deltaTime);
		return;
	}

	glm::vec3 exitTarget = AiSystemHelperFunctions::GetBoxingGloveZoneExitPoint();
	glm::vec3 toExit = exitTarget - transform.position;
	toExit.y = 0.0f;
	float steer = AiSystemHelperFunctions::CalculateSteeringAngle(forward, toExit);

	if (ai.repathCooldown > 0.0f)
		ai.repathCooldown -= deltaTime;

	if (ai.spinnerJumpCooldown > 0.0f)
		ai.spinnerJumpCooldown -= deltaTime;

	// Check if the vehicle's local up vector is pointing downward (flipped over)
	glm::vec3 vehicleUp = transform.quatRotation * glm::vec3(0.0f, 1.0f, 0.0f);
	if (vehicleUp.y < 0.1f) // up vector pointing sideways or downward
	{
		TransitionToState(entity, AiState::IsFlipped, deltaTime);
		return;
	}
	else
	{
		ai.flippedTimer = 0.0f;
	}

	// check if car is far below the nearest navmesh surface
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
		TransitionToState(entity, AiState::RecoveringFromOffTrack, deltaTime);
		return;
	}

	// If we've passed all indexed gloves, just drive toward the exit
	if (ai.currentBoxingGloveIndex >= static_cast<uint32_t>(boxingGloveDangerZones.size()))
	{
		vc.throttle = 0.5f;
		vc.brake = 0.0f;
		vc.steer = steer;
		vc.isGrounded = true;
		return;
	}

	// Get the SPECIFIC danger zone we're targeting by index
	Entity targetDzEntity = boxingGloveDangerZones[ai.currentBoxingGloveIndex];
	auto& dz = controller.GetComponent<DangerZone>(targetDzEntity);

	// Calculate distance to THIS specific danger zone's AABB
	glm::vec3 minBounds = dz.center - dz.halfExtents;
	glm::vec3 maxBounds = dz.center + dz.halfExtents;
	glm::vec3 closestPoint;
	closestPoint.x = glm::clamp(transform.position.x, minBounds.x, maxBounds.x);
	closestPoint.y = glm::clamp(transform.position.y, minBounds.y, maxBounds.y);
	closestPoint.z = glm::clamp(transform.position.z, minBounds.z, maxBounds.z);
	float distToDangerZone = glm::length(transform.position - closestPoint);

	// Check THIS specific glove's arm state
	bool armIsExtending = false;
	if (dz.obstacleEntity != 0 && controller.HasComponent<MovingObstacle>(dz.obstacleEntity))
	{
		auto& obstacle = controller.GetComponent<MovingObstacle>(dz.obstacleEntity);
		armIsExtending = (obstacle.currentPathIndex == 0 || obstacle.currentPathIndex == 1);
	}

	// Three-zone approach for approaching THIS specific glove
	const float APPROACH_DISTANCE = 15.0f;
	const float COAST_DISTANCE = 6.0f;
	const float TRANSITION_DISTANCE = 5.0f;

	if (distToDangerZone > APPROACH_DISTANCE) {
		// Far from targeted glove - drive toward exit with steering
		if(AiSystemHelperFunctions::ShouldLog(m_stateMachineLogTimer, 0.3f, deltaTime)) {
			std::cout << "[AI] Driving toward glove " << ai.currentBoxingGloveIndex
				<< " (dist=" << distToDangerZone << ")" << std::endl;
		}
		vc.throttle = 0.35f;
		vc.brake = 0.0f;
		vc.steer = steer;
		vc.isGrounded = true;
		return;
	}
	else if (distToDangerZone > COAST_DISTANCE) {
		// BUFFER ZONE - coast
		vc.throttle = 0.0f;
		vc.brake = 0.0f;
		vc.steer = steer;
		vc.isGrounded = true;
		return;
	}
	else if (distToDangerZone > TRANSITION_DISTANCE) {
		if (armIsExtending) {
			// ARM IS EXTENDING - WAIT HERE
			vc.throttle = 0.0f;
			vc.brake = 0.0f;
			vc.steer = 0.0f;
			vc.isGrounded = true;
			return;
		}
		else {
			// ARM IS RETRACTING - safe to proceed
			TransitionToState(entity, AiState::WaitingAtDangerZone, deltaTime);
			return;
		}
	}
	else {
		// Very close to or inside the danger zone
		if (armIsExtending) {
			vc.throttle = 0.0f;
			vc.brake = 0.0f;
			vc.steer = 0.0f;
			vc.isGrounded = true;
			return;
		}
		else {
			TransitionToState(entity, AiState::WaitingAtDangerZone, deltaTime);
			return;
		}
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
		TransitionToState(entity, AiState::FollowPath, deltaTime);
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
		TransitionToState(entity, AiState::FollowPath, deltaTime);
	}
}

void AiSystem::UpdateFloorItState(Entity entity, float deltaTime)
{
	TransitionToState(entity, AiState::FollowPath, deltaTime);
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
		TransitionToState(entity, AiState::FollowPath, deltaTime);
		return;
	}

	// Check if the powerup entity still exists
	if (!controller.HasComponent<Powerup>(ai.targetPowerupEntity) ||
		!controller.HasComponent<Transform>(ai.targetPowerupEntity))
	{
		std::cout << "[AI] Powerup gone, resuming path" << std::endl;
		ai.targetPowerupEntity = 0;
		RecomputeNavPath(entity);
		TransitionToState(entity, AiState::FollowPath, deltaTime);
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
		TransitionToState(entity, AiState::FollowPath, deltaTime);
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
		TransitionToState(entity, AiState::FollowPath, deltaTime);
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

	// If we've landed upright, cancel the flip — no reset needed
		glm::vec3 vehicleUp = transform.quatRotation * glm::vec3(0.0f, 1.0f, 0.0f);
	if (vehicleUp.y > 0.5f && vc.isGrounded)
	{
		ai.flippedTimer = 0.0f;
		TransitionToState(entity, AiState::FollowPath, deltaTime);
		return;
	}

	if (vc.isGrounded)
		ai.flippedTimer += deltaTime;
	else
		ai.flippedTimer = 0.0f;  // Reset timer while airborne

	if (ai.flippedTimer > ai.flippedTimeThreshold)
	{
		std::cout << "[AI] Flipped over, resetting vehicle" << std::endl;
		ai.flippedTimer = 0.0f;

		Event resetEvent(Events::Player::RESET_VEHICLE);
		resetEvent.SetParam<Entity>(Events::Player::Reset_Vehicle::ENTITY, entity);
		controller.SendEvent(resetEvent);

		ai.targetPowerupEntity = 0;
		ai.seekTimer = 0.0f;

		// Defer repath to next frame — Transform won't be synced from PhysX until
		// PhysicsSystem::Update runs, so repathing now would use the old position
		ai.needsRepath = true;
		ai.isBypassingSpinner = false;
		ai.needsRepath = true;
		TransitionToState(entity, AiState::FollowPath, deltaTime);
		return;
	}
}

void AiSystem::UpdateUsePowerupState(Entity entity, float deltaTime)
{
	// Powerup usage is now handled inline in FollowPath via TryUsePowerup.
	// If we somehow end up here, just go back to FollowPath.
	TransitionToState(entity, AiState::FollowPath, deltaTime);
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

	TransitionToState(entity, AiState::FollowPath, deltaTime);
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

	TransitionToState(entity, AiState::FollowPath, deltaTime);
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