#pragma once

#include <array>
#include <cassert>
#include <unordered_map>

#include "../Core/Types.h"

struct VehicleCommands
{
	int score;
	float throttle;
	float brake;
	float steer;
	bool isGrounded;
};
