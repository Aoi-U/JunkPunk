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