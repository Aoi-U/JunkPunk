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
	float lookaheadDistance = 5.0f;

	// Steering PD
	float maxSteerRadians = 1.0f;
	float steerDeadzoneDot = 0.98f;

	// Speed control
	float maxSpeed = 50.0f;         // top speed on straight sections
	float cornerSpeed = 4.0f;       // slowest speed at hairpins (increase if the AI is too cautious)
	float brakingDistance = 25.0f;   // how far ahead to start slowing down for a corner
	int lookaheadWaypoints = 8;     // how many waypoints ahead to scan for turns

	// Throttle / brake
	float throttleKp = 1.5f;
	float brakeKp = 10.0f;           // brake aggressiveness (increase if the AI doesn't slow down fast enough)
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

	// Re-path cooldown
	float repathCooldown = 0.0f;
	float repathCooldownDuration = 3.0f;

	// Obstacle avoidance
	float obstacleDetectionRange = 10.0f;
	float avoidanceSteerMultiplier = 1.5f;
	glm::vec3 detectedObstaclePosition = glm::vec3(0.0f);

	// Off-track recovery
	float offTrackHeightThreshold = 8.0f;
	float recoveryTimer = 0.0f;

	// Seek powerup
	float powerupSeekRange = 20.0f;

	// Overtaking
	float overtakeDetectionRange = 15.0f;
	float overtakeSteerOffset = 2.0f;
};