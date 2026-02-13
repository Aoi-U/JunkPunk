#include "Time.h"

Time::Time()
{
	currentTime = glfwGetTime();
}

void Time::Update()
{
	float newTime = glfwGetTime();
	frameTime = newTime - currentTime;
	currentTime = newTime;

	if (paused)
		return;

	accumulator += frameTime;

	totalTime += frameTime;
}