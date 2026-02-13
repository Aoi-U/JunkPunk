#pragma once

#include <array>
#include <cassert>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../Core/Types.h"

struct VehicleCommands
{
	int score;
	float throttle;
	float brake;
	float steer;
	bool isGrounded;
};

struct CheckPoint
{
	glm::quat orientation; // the direction the player should face when player resets their vehicle to checkpoint
};