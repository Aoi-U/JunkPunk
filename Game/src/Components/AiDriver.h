#pragma once

#include <array>
#include <cassert>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

#include "../Core/Types.h"

enum class AiState
{
	FollowPath,
	AvoidObstacle,
	BackingUp,
	Overtaking,
	RecoveringFromOffTrack,
	SeekPowerup,
	UsePowerup,
	Braking
};

struct AiDriver
{
	// Runtime state
	std::uint32_t currentWaypointIndex = 0;
	AiState currentState = AiState::FollowPath;
	AiState previousState = AiState::FollowPath;

	// Navmesh path waypoints (set by AiSystem after pathfinding)
	std::vector<glm::vec3> navWaypoints;

	// Tuning (per-entity)
	float arrivalRadius = 5.0f;
	float desiredSpeed = 12.0f;
	float lookaheadDistance = 5.0f;

	// Steering PD
	float maxSteerRadians = 1.0f;
	float steerDeadzoneDot = 0.98f;

	// Throttle / brake
	float throttleKp = 1.5f; // `throttleKp = 0.5` means the car ramps up slowly.Increasing it makes the AI hit target speed faster
	float brakeAngleThreshold = 0.9f;
	float maxThrottle = 1.0f;
	float maxBrake = 1.0f;

	// Stuck detection and recovery
	float stuckTimer = 0.0f;
	float stuckTimeThreshold = 1.0f;
	float stuckSpeedThreshold = 1.5f;
	float backupTimer = 0.0f;
	float backupDuration = 0.5f;

	// Obstacle avoidance
	float obstacleDetectionRange = 10.0f;
	float avoidanceSteerMultiplier = 1.5f;
	glm::vec3 detectedObstaclePosition = glm::vec3(0.0f);

	// Off-track recovery
	float maxDistanceFromTrack = 30.0f;
	float recoveryTimer = 0.0f;

	// Braking
	float brakingAngleThreshold = 0.7f; // dot product below this triggers braking
	int brakingLookaheadWaypoints = 3;   // how many waypoints ahead to check for sharp turns
	float brakingSpeed = 4.0f;           // target speed when braking

	// Seek powerup
	float powerupSeekRange = 20.0f;      // max distance to detour for a powerup

	// Overtaking
	float overtakeDetectionRange = 15.0f;
	float overtakeSteerOffset = 2.0f;
};