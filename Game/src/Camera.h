#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include "InputManager.h"

class Camera
{
public:
	struct Params
	{
		float defaultRadius = 0.0f;
		float defaultTheta = 0.0f;
		float defaultPhi = 0.0f;

		float defaultDistance = 2.0f;
		float minDistance = 2.0f; // distance the camera is at when the car is idle
		float maxDistance = 10.0f; // distance the camera is at when the car is at max speed
	};

	// Angles are in radians
	Camera(glm::mat4& target, const Params& params);

	void ChangeTheta(float deltaTheta);

	void ChangePhi(float deltaPhi);

	void ChangeRadius(float deltaRadius);

	// probably need some function to lerp between current and new position for smooth camera movement

	void Reset();

	glm::mat4 GetViewMatrix();

	glm::vec3 GetPosition();

	float GetDistance();

	float GetTheta();

	float GetPhi();

private:

	void UpdateViewMatrix();

	glm::mat4* target;

	float minDistance;
	float maxDistance;
	float distance;

	float defaultTheta;
	float defaultPhi;
	float defaultDistance;

	float theta;
	float phi;

	bool isDirty = true;

	glm::mat4 viewMatrix{};
	glm::vec3 position{};
};
