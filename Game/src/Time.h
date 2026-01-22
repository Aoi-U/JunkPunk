#pragma once

#include "GLFW/glfw3.h"

class Time
{
public:
	Time();

	void Update();

	float getDeltaTime() const { return (float)deltaTime;  }

	float getFPS() const { return 1 / deltaTime; }
	
private:
	double deltaTime;
	double lastFrameTime;
};