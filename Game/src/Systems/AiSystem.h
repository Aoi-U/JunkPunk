#pragma once

#include "System.h"
#include "../NavMesh.h"
#include <glm/glm.hpp>
#include <vector>

enum class AiState;

class Game;

class AiSystemDebug; // forward declaration

class AiSystem : public System
{
	friend class AiSystemDebug; // Allow access to private members
public:
	void Init();
	void Update(float deltaTime);
	void SetGame(Game* game) { gameInstance = game; }
	void SetNavMesh(const NavMesh& mesh);
	const NavMesh& GetNavMesh() const { return navMesh; }

	void SetGoalPosition(const glm::vec3& goal) { goalPosition = goal; }

	// Zone detection helpers (delegated to AiSystemHelperFunctions)
	// These are kept as public convenience wrappers
	float CheckDangerZone(const glm::vec3& position) const;
	bool IsInGapZone(const glm::vec3& pos) const;
	bool IsInTunnelZone(const glm::vec3& pos) const;
	bool IsInBoxingGloveZone(const glm::vec3& pos) const;

	float CalculateDistanceToFinish(const glm::vec3& position) const;
private:
	Game* gameInstance = nullptr;
	NavMesh navMesh;
	glm::vec3 goalPosition = glm::vec3(-70.000f, 56.000f, 326.000f);

	// Logging throttle timers
	float m_stateMachineLogTimer = 0.0f;
	float m_followPathLogTimer = 0.0f;
	float m_waitingLogTimer = 0.0f;

	// Computes A* path from the entity's current position to the goal
	void ComputeNavPath(Entity entity);

	// Re-paths from the entity's current position (used after getting knocked off track)
	void RecomputeNavPath(Entity entity);

	void TransitionToState(Entity entity, AiState newState);

	// State update functions
	void UpdateFollowPathState(Entity entity, float deltaTime);
	void UpdateBackingUpState(Entity entity, float deltaTime);
	void UpdateRecoveringFromOffTrackState(Entity entity, float deltaTime);
	void UpdateAvoidObstacleState(Entity entity, float deltaTime);
	void UpdateBrakingState(Entity entity, float deltaTime);
	void UpdateSeekPowerupState(Entity entity, float deltaTime);
	void UpdateUsePowerupState(Entity entity, float deltaTime);
	void UpdateOvertakingState(Entity entity, float deltaTime);
	void UpdateBoxingGloveZoneState(Entity entity, float deltaTime);
	void UpdateGapZoneState(Entity entity, float deltaTime);
	void UpdateTunnelZoneState(Entity entity, float deltaTime);
	void UpdateWaitingAtDangerZoneState(Entity entity, float deltaTime);
	void UpdateIsFlippedState(Entity entity, float deltaTime);
	void UpdateIsStuckState(Entity entity, float deltaTime);
	void UpdateFloorItState(Entity entity, float deltaTime);
};