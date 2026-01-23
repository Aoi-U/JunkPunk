#pragma once

#include "GLFW/glfw3.h"

class Time
{
public:
	Time();

	void Update();

	float fps() const { return 1 / frameTime; }
	
	float frameTime;
	float currentTime;
	float newTime;
	float accumulator;
	const double deltaTime = 1 / 60.0f;
	float totalTime;
private:
};