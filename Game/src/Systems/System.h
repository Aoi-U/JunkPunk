#pragma once

#include <set>

#include "../Core/Types.h"

// each system should inherit from this system class
class System
{
public:
	std::set<Entity> entities;
};