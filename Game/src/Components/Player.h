#pragma once

#include <array>
#include <cassert>
#include <unordered_map>

#include "../Core/Types.h"

struct Player
{
	int score;
	float throttle;
	float brake;
	float steer;
	bool jump;
};