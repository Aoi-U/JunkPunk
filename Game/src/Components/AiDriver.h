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
	float lookaheadDistance = 15.0f;

	// Steering PD
	float maxSteerRadians = 1.0f;
	float steerDeadzoneDot = 0.98f;

	// Speed -- constant for now
	float desiredSpeed = 15.0f;

	// Throttle
	float throttleKp = 1.5f;
	float maxThrottle = 1.0f;

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
	float avoidanceSteerMultiplier = 1.5f;
	glm::vec3 detectedObstaclePosition = glm::vec3(0.0f);

	// Off-track recovery
	float offTrackHeightThreshold = 8.0f;
	float recoveryTimer = 0.0f;

	// Powerup seeking
	float powerupSeekRange = 50.0f;        // max distance to detour for a powerup
	float powerupSeekMaxAngle = -0.3f;      // dot product threshold -- -0.3 = nearly full 360, excludes directly behind
	//### Reference for `powerupSeekMaxAngle`

	//	| Value | Detection cone |
	//	|-- - | -- - |
	//	| `0.95` | ~18° — nearly straight ahead only |
	//	| `0.5` | ~60° — forward cone |
	//	| `0.0` | ~90° — front hemisphere |
	//	| `-0.3` | ~107° — most directions except directly behind |
	//	| `-1.0` | 360° — any direction, will even turn around |
	Entity targetPowerupEntity = 0;         // entity ID of the powerup we're chasing
	float seekTimer = 0.0f;                 // how long we've been seeking
	float seekTimeout = 6.0f;              // give up after this many seconds
	bool hasPowerup = false;                // whether we currently hold a powerup
	int heldPowerupType = 0;                // type of held powerup (1=speed boost, 2=banana)

	// Powerup usage
	float useBoostDot = 0.95f;             // use speed boost when path is this straight ahead
	float dropBananaPlayerRange = 15.0f;   // drop banana when player is this close behind

	// Overtaking
	float overtakeDetectionRange = 15.0f;
	float overtakeSteerOffset = 2.0f;

	// Flip detection
	float flippedTimer = 0.0f;
	float flippedTimeThreshold = 1.0f;  // seconds upside-down before auto-reset

	// Obstacle avoidance
	float obstacleDetectionRange = 40.0f;   // how far ahead to detect. Increase if the AI reacts too late at high speed.
	float obstacleDetectionCone = 0.5f;     // dot product threshold -- 0.5 = ~60 degree forward cone. Lower to detect obstacles further to the side.
	float avoidanceSteerDirection = 0.0f;   // -1.0 = steer left, 1.0 = steer right (set on detection)
	Entity detectedObstacleEntity = 0;      // entity we're currently avoiding
	float avoidTimer = 0.0f;                // how long we've been avoiding
	float avoidDuration = 6.0f;             // how long to steer away. Increase for larger obstacles like spinning pushers.
	float avoidSteerStrength = 1.0f;        // how hard to steer while avoiding. `1.0` = full lock. Lower for gentler swerves.
	float avoidThrottleScale = 0.3f;        // slow down while avoiding (0.3 = 30% throttle). speed reduction during avoidance. Lower for more cautious dodging.

	// Sequential obstacle passing (for synchronized obstacles like gloves)
	Entity currentObstacleTarget = 0;       // the obstacle we're currently waiting to pass
	bool observedExtended = false;          // have we seen the current target extend?
	float obstacleWaitStopDistance = 35.0f; // stop this far before the obstacle
	float obstacleCommitDistance = 3.0f;    // once this close (INSIDE zone), commit to passing (don't stop again)
};