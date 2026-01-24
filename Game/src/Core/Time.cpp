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
	accumulator += frameTime;

	totalTime += frameTime;
}