#pragma once

#include "System.h"
#include "../NavMesh.h"
#include <glm/glm.hpp>
#include <vector>

enum class AiState;

class AiSystem : public System
{
public:
	bool useAnchors = true; // Toggle: true = anchor-based, false = pure A*

	void Init();
	void Update(float deltaTime);

	void SetNavMesh(const NavMesh& mesh);
	const NavMesh& GetNavMesh() const { return navMesh; }

	void SpawnDebugWaypoints(Entity entity);

private:
	NavMesh navMesh;

	std::vector<glm::vec3> raceAnchors;
	void InitializeRaceAnchors();

	// Single entry point — delegates based on useAnchors flag
	void ComputeNavPath(Entity entity, size_t anchorStartIndex = 0);
	void ComputeNavPathWithAnchors(Entity entity, size_t anchorStartIndex);
	void ComputeNavPathWithoutAnchors(Entity entity);

	size_t FindNearestAnchorIndex(const glm::vec3& position) const;

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

	float CalculateSteeringAngle(const glm::vec3& forward, const glm::vec3& toTarget);
};