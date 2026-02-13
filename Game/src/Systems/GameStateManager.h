#pragma once

#include <memory>
#include "System.h"
#include "../Core/Types.h"
#include "../Event.h"
#include "../../Gamepad.h"

class Event;

class GameStateControlSystem : public System
{
public:
	GameStateControlSystem();

	void Update();

private:
	std::shared_ptr<Gamepad> gamepad;
};