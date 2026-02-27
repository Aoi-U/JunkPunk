#pragma once

#include <array>
#include <cassert>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../Core/Types.h"

struct AiDriver
{
	// Runtime state
	std::uint32_t currentWaypointIndex = 0;

	// Tuning (per-entity)
	float arrivalRadius = 2.0f;
	float desiredSpeed = 12.0f;

	// Steering PD
	float maxSteerRadians = 1.0f;

	// Throttle / brake
	float throttleKp = 0.5f;
	float brakeAngleThreshold = 0.9f;
	float maxThrottle = 1.0f;
	float maxBrake = 1.0f;
};