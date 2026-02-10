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
	void Init(std::shared_ptr<Gamepad> gamepad);

	void Update(float deltaTime);

private:
	void WindowSizeListener(Event& e);

	void MouseMovedListener(Event& e);

	void BoostCamera(ThirdPersonCamera& camera, float deltaTime);

	void ReturnCamera(ThirdPersonCamera& camera, float deltaTime);

	std::shared_ptr<Gamepad> gamepad;
	int screenWidth = 1280;
	int screenHeight = 720;

	double lastPosX;
	double lastPosY;
};