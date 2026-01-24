#pragma once
#include <memory>
#include <iostream>

#include "../Systems/System.h"
#include "../../Gamepad.h"

class Event;

class Gamepad;

class CameraControlSystem : public System
{
public:
	void Init(std::shared_ptr<Gamepad> gamepad);

	void Update(float deltaTime);


private:
	void WindowSizeListener(Event& e);

	std::shared_ptr<Gamepad> gamepad;
	int screenWidth = 1280;
	int screenHeight = 720;
};