#pragma once

#include "GLFW/glfw3.h"

class Time
{
public:
	Time();

	void Update();

	double getDeltaTime() const { return deltaTime;  }

private:
	double deltaTime;
	double lastFrameTime;
};