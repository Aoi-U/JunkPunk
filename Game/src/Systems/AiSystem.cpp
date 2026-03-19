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

	for (auto entity : entities)
	{
		if (trackWaypoints.empty())
			return;

		UpdateStateMachine(entity, deltaTime);
	}
}

void AiSystem::InitializeWaypoints()
{
	trackWaypoints.clear();

	std::vector<glm::vec3> anchors{
		glm::vec3(-39.0f, -94.0f, -8.0f), // Waypoint near computer opponent
		glm::vec3(-80.0f, -93.0f, 19.0f),
		glm::vec3(-49.266, -84.591, 49.396),
		glm::vec3(-5, -70.691, 5),
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
	};

	int nBetween = 5;

	// Build trackWaypoints: include first anchor, then for each segment add nBetween interior points.
	if (anchors.empty())
		return;

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

void AiSystem::TransitionToState(Entity entity, AiState newState)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	ai.previousState = ai.currentState;
	ai.currentState = newState;

	// Reset state-specific timers on entry
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
	//case AiState::AvoidObstacle:
	//	UpdateAvoidObstacleState(entity, deltaTime);
	//	break;
	//case AiState::Overtaking:
	//	UpdateOvertakingState(entity, deltaTime);
	//	break;
	}
}

void AiSystem::UpdateFollowPathState(Entity entity, float deltaTime)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);
	auto& body = controller.GetComponent<VehicleBody>(entity);
	auto& vc = controller.GetComponent<VehicleCommands>(entity);

	glm::vec3 positionXZ(transform.position.x, 0.0f, transform.position.z);
	glm::vec3 waypointPosition = trackWaypoints[ai.currentWaypointIndex].position;
	glm::vec3 waypointXZ(waypointPosition.x, 0.0f, waypointPosition.z);

	float dist = glm::length(waypointXZ - positionXZ);

	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	int prevIndex = (ai.currentWaypointIndex - 1 + trackWaypoints.size()) % trackWaypoints.size();
	glm::vec3 prevPos = trackWaypoints[prevIndex].position;
	glm::vec3 prevXZ(prevPos.x, 0.0f, prevPos.z);

	glm::vec3 segmentDir = glm::normalize(waypointXZ - prevXZ);
	glm::vec3 toCar = positionXZ - prevXZ;
	float t = glm::dot(toCar, segmentDir);

	float segmentLength = glm::length(waypointXZ - prevXZ);
	if (t + ai.lookaheadDistance > segmentLength)
	{
		ai.currentWaypointIndex = (ai.currentWaypointIndex + 1) % trackWaypoints.size();
	}

	float lookahead = ai.lookaheadDistance;
	glm::vec3 lookaheadPoint = prevXZ + segmentDir * (t + lookahead);

	glm::vec3 toTarget = lookaheadPoint - waypointXZ;
	glm::vec3 toTargetN = glm::length(toTarget) > 1e-5f ? glm::normalize(toTarget) : forward;

	float steer = 0.0f;
	float forwardDot = glm::clamp(glm::dot(forward, toTargetN), -1.0f, 1.0f);
	if (std::abs(forwardDot) > ai.steerDeadzoneDot)
	{
		steer = 0.0f;
	}
	else
	{
		steer = CalculateSteeringAngle(forward, toTarget);
	}

	float targetSpeed = (trackWaypoints[ai.currentWaypointIndex].recommendedSpeed > 0.0f)
		? trackWaypoints[ai.currentWaypointIndex].recommendedSpeed
		: ai.desiredSpeed;
	float speed = glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));
	float baseThrottle = glm::clamp((targetSpeed - speed) * ai.throttleKp, 0.0f, ai.maxThrottle);

	// --- Stuck detection: transition to BackingUp ---
	if (speed < ai.stuckSpeedThreshold && baseThrottle > 0.5f)
	{
		ai.stuckTimer += deltaTime;
		if (ai.stuckTimer > ai.stuckTimeThreshold)
		{
			TransitionToState(entity, AiState::BackingUp);
			return; // Exit early, BackingUp will run next frame
		}
	}
	else
	{
		ai.stuckTimer = 0.0f;
	}

	// Normal driving commands
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

	// Done backing up — return to path following
	if (ai.backupTimer >= ai.backupDuration)
	{
		TransitionToState(entity, AiState::FollowPath);
		return;
	}

	// Steer toward the next waypoint, but inverted for reverse
	glm::vec3 positionXZ(transform.position.x, 0.0f, transform.position.z);
	glm::vec3 waypointPos = trackWaypoints[ai.currentWaypointIndex].position;
	glm::vec3 waypointXZ(waypointPos.x, 0.0f, waypointPos.z);

	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	glm::vec3 toTarget = waypointXZ - positionXZ;

	glm::vec2 fwd2D(forward.x, forward.z);
	glm::vec2 target2D(toTarget.x, toTarget.z);
	if (glm::length(target2D) > 1e-5f)
		target2D = glm::normalize(target2D);

	float cross = fwd2D.x * target2D.y - fwd2D.y * target2D.x;
	float dot = glm::clamp(fwd2D.x * target2D.x + fwd2D.y * target2D.y, -1.0f, 1.0f);
	float angle = std::atan2(cross, dot);

	// Aggressive gain (1.0 instead of 0.2) — gives full lock steering immediately
	float steer = glm::clamp(angle * 1.0f, -1.0f, 1.0f);

	vc.steer = -steer;    // Invert for reverse
	vc.throttle = 0.0f;
	vc.brake = 1.0f;
	vc.isGrounded = true;
}
//
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

	return glm::clamp(angle * 0.2f, -1.0f, 1.0f);
}

void AiSystem::UpdateRecoveringFromOffTrackState(Entity entity, float deltaTime)
{
	// TODO: Implement off-track recovery
	TransitionToState(entity, AiState::FollowPath);
}
//
//void AiSystem::UpdateAvoidObstacleState(Entity entity, float deltaTime)
//{
//	// TODO: Implement obstacle avoidance
//	TransitionToState(entity, AiState::FollowPath);
//}
//
//void AiSystem::UpdateOvertakingState(Entity entity, float deltaTime)
//{
//	// TODO: Implement overtaking
//	TransitionToState(entity, AiState::FollowPath);
//}
//