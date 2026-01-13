#include "Time.h"

Time::Time()
{
	lastFrameTime = glfwGetTime();
	deltaTime = 0.0;
}

void Time::Update()
{
	double currentFrameTime = glfwGetTime();
	deltaTime = currentFrameTime - lastFrameTime;
	lastFrameTime = currentFrameTime;
}