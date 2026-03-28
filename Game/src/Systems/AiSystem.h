#pragma once

#include "System.h"
#include "../NavMesh.h"
#include <glm/glm.hpp>
#include <vector>

enum class AiState;

struct ObstacleDangerZone
{
	glm::vec3 center;     // center of the obstacle's movement range
	float radius;         // radius covering entire movement range
	Entity obstacleEntity; // the actual obstacle entity
};

class AiSystem : public System
{
public:
	void Init();
	void Update(float deltaTime);

	void SetNavMesh(const NavMesh& mesh);
	const NavMesh& GetNavMesh() const { return navMesh; }

	void SetGoalPosition(const glm::vec3& goal) { goalPosition = goal; }

	void SpawnDebugWaypoints(Entity entity);
	// Spawns a 1x1 trigger box at every navmesh triangle centroid (node)
	void SpawnDebugNodes();

	// Build danger zones from all MovingObstacle entities (call after level load)
	void BuildObstacleDangerZones();

	// Check if a position is in a danger zone (returns closest danger distance, or -1 if safe)
	float CheckDangerZone(const glm::vec3& position) const;

private:
	NavMesh navMesh;
	glm::vec3 goalPosition = glm::vec3(25.0f, -3.5f, 120.0f);
	std::vector<ObstacleDangerZone> dangerZones;

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

	// Checks if conditions are right to use a held powerup. Called during FollowPath.
	void TryUsePowerup(Entity entity);

	float CalculateSteeringAngle(const glm::vec3& forward, const glm::vec3& toTarget);
};