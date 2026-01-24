#pragma once

#include <memory>

#include "System.h"
#include "../../Gamepad.h"

class VehicleControlSystem : public System
{
public:
	VehicleControlSystem() = default;
	void Init(std::shared_ptr<Gamepad> gamepad);
	void Update();

private:
		enum Buttons
	{
		JUMP			=		0, // A
		POWERUP		=		1, // B
		X					=		2, // X not using yet
		Y					=		3, // Y not using yet
		LEFTROLL	=		8, // LB
		RIGHTROLL	=		9, // RB
		PAUSE			=		12, // START
	};
	std::shared_ptr<Gamepad> gamepad;
};