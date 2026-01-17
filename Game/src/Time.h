#pragma once

#include "GLFW/glfw3.h"

class Time
{
public:
	Time();

	void Update();

	float getDeltaTime() const { return (float)deltaTime;  }
	
private:
	double deltaTime;
	double lastFrameTime;
};