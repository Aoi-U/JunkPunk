#pragma once
#include <memory>
#include <iostream>

#include "System.h"
#include "../../Gamepad.h"
#include "../Event.h"
#include "../Components/Camera.h"

class Gamepad;

class CameraControlSystem : public System
{
public:
	void Init(std::vector<std::shared_ptr<Gamepad>> gamepads);

	void Update(float deltaTime);

private:
	void WindowSizeListener(Event& e);

	void MouseMovedListener(Event& e);

	//std::shared_ptr<Gamepad> gamepad;
	std::unordered_map<int, std::shared_ptr<Gamepad>> gamepads; // map of player entities to their gamepads 
	int screenWidth = 1280;
	int screenHeight = 720;

	double lastPosX;
	double lastPosY;
};