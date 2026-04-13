#pragma once

#include <glm/glm.hpp>
#include "../Core/Types.h"

// Forward declarations
class AiSystem;

/**
 * @brief Static utility functions for AI system
 *
 * These helper functions provide various utilities for the AI system including:
 * - Danger zone detection and distance calculations
 * - Zone-based position checks (gap, tunnel, boxing glove zones)
 * - Steering angle calculations
 * - Powerup usage logic
 * - Logging helpers
 */
class AiSystemHelperFunctions
{
public:
	// ===== LOGGING HELPERS =====

	/**
	 * @brief Returns true when interval has elapsed, auto-resets timer
	 * @param timer Reference to timer variable (will be modified)
	 * @param interval Time interval to check against
	 * @param deltaTime Time since last frame
	 * @return true if interval has elapsed, false otherwise
	 */
	static bool ShouldLog(float& timer, float interval, float deltaTime);

	// ===== DANGER ZONE HELPERS =====

	/**
	 * @brief Check if a position is in a danger zone
	 * @param position Position to check
	 * @return Closest danger distance, or -1 if safe
	 */
	static float CheckDangerZone(const glm::vec3& position);

	/**
	 * @brief Returns true if the point is inside any DangerZone (regardless of obstacle state)
	 */
	static bool IsPointInDangerZone(const glm::vec3& point);

	/**
	 * @brief Returns true if the point is inside any DangerZone whose glove is currently extended
	 */
	static bool IsPointInActiveDangerZone(const glm::vec3& point);

	/**
	 * @brief Returns true if the point is in a danger zone AND the obstacle is physically blocking
	 */
	static bool IsObstacleInDangerZone(const glm::vec3& point);

	/**
	 * @brief Check if an obstacle entity has an associated danger zone
	 */
	static bool HasDangerZone(Entity obstacleEntity);

	/**
	 * @brief Get distance from a point to the nearest danger zone
	 */
	static float GetDistanceToDangerZone(const glm::vec3& point);
	static glm::vec3 GetBoxingGloveZoneExitPoint();

	/**
	 * @brief Check if a boxing glove arm is currently blocking at this position
	 */
	static bool IsArmBlocking(const glm::vec3& carPos);
	static bool IsBoxingGloveArmExtending(const glm::vec3& carPos, float checkDistance);

	/**
	 * @brief Scan for danger zones in a forward cone
	 * @param carPos Current position of the car
	 * @param forward Normalized forward direction vector
	 * @param detectionRange Maximum distance to scan
	 * @param detectionCone Dot product threshold for cone (e.g., 0.7 = ~45° cone)
	 * @param outClosestDangerEntity Optional output parameter for closest danger zone entity
	 * @return Distance to closest danger zone in cone, or -1 if none found
	 */
	static float ScanForDangerZoneInCone(const glm::vec3& carPos, const glm::vec3& forward, 
		float detectionRange, float detectionCone, Entity* outClosestDangerEntity = nullptr);

	// ===== ZONE DETECTION HELPERS =====

	/**
	 * @brief Check if position is in the boxing glove zone
	 */
	static bool IsInBoxingGloveZone(const glm::vec3& pos);

	/**
	 * @brief Scan for boxing glove zone in forward cone
	 * @return Distance to boxing glove zone, or -1 if not in cone
	 */
	static float ScanForBoxingGloveZoneInCone(const glm::vec3& carPos, const glm::vec3& forward,
		float detectionRange, float detectionCone);

	/**
	 * @brief Check if position is in the gap zone
	 */
	static bool IsInGapZone(const glm::vec3& pos);

	/**
	 * @brief Scan for gap zone in forward cone
	 * @return Distance to gap zone, or -1 if not in cone
	 */
	static float ScanForGapZoneInCone(const glm::vec3& carPos, const glm::vec3& forward,
		float detectionRange, float detectionCone);

	/**
	 * @brief Check if position is in the tunnel zone
	 */
	static bool IsInTunnelZone(const glm::vec3& pos);

	/**
	 * @brief Scan for tunnel zone in forward cone
	 * @return Distance to tunnel zone, or -1 if not in cone
	 */
	static float ScanForTunnelZoneInCone(const glm::vec3& carPos, const glm::vec3& forward,
		float detectionRange, float detectionCone);

	// ===== UTILITY FUNCTIONS =====

	/**
	 * @brief Calculate steering angle from forward vector to target direction
	 * @param forward Current forward direction (normalized)
	 * @param toTarget Direction to target
	 * @return Steering angle clamped to [-1, 1]
	 */
	static float CalculateSteeringAngle(const glm::vec3& forward, const glm::vec3& toTarget);

	// ===== ACTIONS =====

	/**
	 * @brief Advance AI entity through the boxing glove zone
	 * @note Currently unused - placeholder for future implementation
	 */
	static void AdvanceThroughBoxingGlove(Entity entity);

	/**
	 * @brief Try to use a held powerup if conditions are right
	 * @param entity AI entity attempting to use powerup
	 * @param gameInstance Pointer to Game instance (for spawning bananas)
	 */
	static void TryUsePowerup(Entity entity, class Game* gameInstance);

	static float GetCurrentSpeed(Entity entity);
};
