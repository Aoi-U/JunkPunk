#pragma once

#include <memory>

#include "System.h"
#include "../../Gamepad.h"
#include "../Core/Types.h"
#include "../Event.h"


class VehicleControlSystem : public System
{
public:
	VehicleControlSystem() = default;
	void Init(std::shared_ptr<Gamepad> gamepad);
	void Update();

private:
	void KeyboardInputListener(Event& e);

	enum Buttons
	{
		JUMP			=		0, // A
		POWERUP		=		1, // B
		X					=		2, // X not using yet
		Y					=		3, // Y not using yet
		LEFTROLL	=		8, // LB
		RIGHTROLL	=		9, // RB
		PAUSE			=		12, // START
		RESET			=		13  // BACK
	};
		
	enum Keys
	{
		KEY_FORWARD		= 'W',
		KEY_BACKWARD	= 'S',
		KEY_LEFT			= 'A',
		KEY_RIGHT			= 'D',
		KEY_JUMP			= ' ',
		KEY_RESET			= 'R'
	};

	/*	A = 0;
		B = 1;
		X = 2;
		Y = 3;

		DPad_Up = 4;
		DPad_Down = 5;
		DPad_Left = 6;
		DPad_Right = 7;

		L_Shoulder = 8;
		R_Shoulder = 9;

		L_Thumbstick = 10;
		R_Thumbstick = 11;

		Start = 12;
		Back = 13;*/
	std::shared_ptr<Gamepad> gamepad;
};