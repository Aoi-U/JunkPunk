#pragma once

#include "System.h"
#include "../NavMesh.h"
#include <glm/glm.hpp>
#include <vector>

enum class AiState;

class Game;

class AiSystem : public System
{
public:
	void Init();
	void Update(float deltaTime);
	void SetGame(Game* game) { gameInstance = game; }
	void SetNavMesh(const NavMesh& mesh);
	const NavMesh& GetNavMesh() const { return navMesh; }

	void SetGoalPosition(const glm::vec3& goal) { goalPosition = goal; }

	void SpawnDebugWaypoints(Entity entity);
	// Spawns a 1x1 trigger box at every navmesh triangle centroid (node)
	void SpawnDebugNodes();

	// Check if a position is in a danger zone (returns closest danger distance, or -1 if safe)
	float CheckDangerZone(const glm::vec3& position) const;

private:
	Game* gameInstance = nullptr;
	NavMesh navMesh;
	glm::vec3 goalPosition = glm::vec3(25.0f, -3.5f, 120.0f);

	// Computes A* path from the entity's current position to the goal
	void ComputeNavPath(Entity entity);

	// Re-paths from the entity's current position (used after getting knocked off track)
	void RecomputeNavPath(Entity entity);

	void UpdateStateMachine(Entity entity, float deltaTime);
	void TransitionToState(Entity entity, AiState newState);

	void UpdateFollowPathState(Entity entity, float deltaTime);
	void UpdateBackingUpState(Entity entity, float deltaTime);
	void UpdateRecoveringFromOffTrackState(Entity entity, float deltaTime);
	void UpdateAvoidObstacleState(Entity entity, float deltaTime);
	void UpdateBrakingState(Entity entity, float deltaTime);
	void UpdateSeekPowerupState(Entity entity, float deltaTime);
	void UpdateUsePowerupState(Entity entity, float deltaTime);
	void UpdateOvertakingState(Entity entity, float deltaTime);
	void UpdateWaitingAtDangerZoneState(Entity entity, float deltaTime);
	bool IsObstacleInDangerZone(const glm::vec3& point) const;
	void AdvanceThroughBoxingGlove(Entity entity);

	// Checks if conditions are right to use a held powerup. Called during FollowPath.
	void TryUsePowerup(Entity entity);

	// Returns true if the point is inside any DangerZone whose glove is currently extended
	bool IsPointInActiveDangerZone(const glm::vec3& point) const;
	bool IsPointInDangerZone(const glm::vec3& point) const;

	bool HasDangerZone(Entity obstacleEntity) const;

	float GetDistanceToDangerZone(glm::vec3 carPos);

	bool IsArmBlocking(glm::vec3 carPos);

	float CalculateSteeringAngle(const glm::vec3& forward, const glm::vec3& toTarget);
};