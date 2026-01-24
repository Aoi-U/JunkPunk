#pragma once

#include "GLFW/glfw3.h"

class Time
{
public:
	Time();

	void Update();

	float fps() const { return 1 / frameTime; }
	
	float totalTime = 0.0f;
	const float deltaTime = 1.0f / 60.0f;

	float currentTime = 0.0f;
	float previousTime = glfwGetTime();

	float accumulator = 0.0f;
	float frameTime = 0.0f;
private:
};