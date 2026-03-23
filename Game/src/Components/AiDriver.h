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
	float throttleKp = 1.5f;
	float brakeAngleThreshold = 0.9f;
	float maxThrottle = 1.0f;
	float maxBrake = 1.0f;

	// Stuck detection and recovery
	float stuckTimer = 0.0f;
	float stuckTimeThreshold = 1.0f;
	float stuckSpeedThreshold = 1.5f;
	float backupTimer = 0.0f;
	float backupDuration = 0.5f;

	// Progress tracking
	float progressTimer = 0.0f;
	float progressTimeThreshold = 2.0f;
	float lastDistToWaypoint = 999999.0f;

	// Re-path cooldown to prevent infinite re-pathing
	float repathCooldown = 0.0f;
	float repathCooldownDuration = 3.0f; // minimum seconds between re-paths

	// Obstacle avoidance
	float obstacleDetectionRange = 10.0f;
	float avoidanceSteerMultiplier = 1.5f;
	glm::vec3 detectedObstaclePosition = glm::vec3(0.0f);

	// Off-track recovery
	float offTrackHeightThreshold = 8.0f;
	float recoveryTimer = 0.0f;

	// Braking
	float brakingAngleThreshold = 0.7f;
	int brakingLookaheadWaypoints = 3;
	float brakingSpeed = 4.0f;

	// Seek powerup
	float powerupSeekRange = 20.0f;

	// Overtaking
	float overtakeDetectionRange = 15.0f;
	float overtakeSteerOffset = 2.0f;
};