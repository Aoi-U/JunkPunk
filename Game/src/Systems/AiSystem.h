
#include "System.h"
#include <glm/glm.hpp>
#include <vector>

enum class AiState;

struct Waypoint {
	glm::vec3 position;
	float recommendedSpeed = 10.0f;
	float trackWidth = 5.0f;
};

class AiSystem : public System
{
public:
	void Init();
	void Update(float deltaTime);
	void RenderDebugWaypoints();

	const std::vector<Waypoint>& GetWaypoints() const { return trackWaypoints; }

private:
	std::vector<Waypoint> trackWaypoints;

	void InitializeWaypoints();

	// State machine methods
	void UpdateStateMachine(Entity entity, float deltaTime);
	void TransitionToState(Entity entity, AiState newState);

	// State update methods
	void UpdateFollowPathState(Entity entity, float deltaTime);
	void UpdateAvoidObstacleState(Entity entity, float deltaTime);
	void UpdateBackingUpState(Entity entity, float deltaTime);
	void UpdateOvertakingState(Entity entity, float deltaTime);
	void UpdateRecoveringFromOffTrackState(Entity entity, float deltaTime);

	// Helper methods
	bool DetectObstacle(Entity entity, glm::vec3& obstaclePos);
	bool IsOffTrack(Entity entity);
	float CalculateSteeringAngle(const glm::vec3& forward, const glm::vec3& toTarget);
	glm::vec3 GetLookaheadPoint(Entity entity);
};