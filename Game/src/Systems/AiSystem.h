#pragma once

#include "System.h"
#include "../NavMesh.h"
#include <glm/glm.hpp>
#include <vector>

enum class AiState;

class AiSystem : public System
{
public:
	void Init();
	void Update(float deltaTime);

	// Called by LevelLoaderSystem after the track model is loaded
	void SetNavMesh(const NavMesh& mesh);
	const NavMesh& GetNavMesh() const { return navMesh; }

	// Spawn visible debug markers at each navmesh waypoint for the given entity
	void SpawnDebugWaypoints(Entity entity);

private:
	NavMesh navMesh;
	glm::vec3 goalPosition = glm::vec3(25.0f, -3.5f, 120.0f); // finish line

	// Race anchor points — the AI pathfinds segment by segment through these
	std::vector<glm::vec3> raceAnchors;
	void InitializeRaceAnchors();

	// Computes a navmesh path for an entity through all race anchors
	void ComputeNavPath(Entity entity);

	// State machine methods
	void UpdateStateMachine(Entity entity, float deltaTime);
	void TransitionToState(Entity entity, AiState newState);

	// State update methods
	void UpdateFollowPathState(Entity entity, float deltaTime);
	void UpdateBackingUpState(Entity entity, float deltaTime);
	void UpdateRecoveringFromOffTrackState(Entity entity, float deltaTime);

	// Helper methods
	float CalculateSteeringAngle(const glm::vec3& forward, const glm::vec3& toTarget);
};