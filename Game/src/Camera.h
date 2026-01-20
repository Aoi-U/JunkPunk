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

		float lerpSpeed = 4.0f; // how fast the camera returns to default position
		float lookSpeed = 6.0f; // how fast the camera rotates based on input
	};

	// Angles are in radians
	Camera(glm::mat4& target, const Params& params, float fov, float aspectRatio);

	void Update(float deltaTime, glm::mat4& newTarget);

	void ChangeAspectRatio(float newAspectRatio);

	void ChangeFov(float newFov);

	void ChangeTheta(float deltaTheta);

	void ChangePhi(float deltaPhi);

	void ChangeRadius(float deltaRadius);

	// probably need some function to lerp between current and new position for smooth camera movement

	void Reset();

	glm::mat4 GetViewProjectionMatrix();

	glm::mat4 GetProjectionMatrix() const { return glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar); }

	glm::mat4 GetViewMatrix() const { return viewMatrix; }

	glm::vec3 GetPosition() const { return position; }

	glm::vec3 GetFrontVector() const { return frontVector; }

	glm::vec3 GetUpVector() const { return upVector; }

	glm::vec3 GetRightVector() const { return rightVector; }

	float GetDistance() const { return distance; }

	float GetTheta() const { return theta; }

	float GetPhi() const { return phi; }

	float GetFarClipPlane() const { return zFar; }

	float GetNearClipPlane() const { return zNear; }

	float GetFov() const { return fov; }

	float GetAspectRatio() const { return aspectRatio; }

private:

	float aspectRatio;

	glm::mat4* target;

	float minDistance;
	float maxDistance;
	float distance;

	float defaultTheta;
	float defaultPhi;
	float defaultDistance;

	float fov;
	float zNear = 0.1f;
	float zFar = 1000.0f;
	float theta;
	float phi;

	float lerpSpeed;
	float lookSpeed;

	glm::mat4 viewMatrix{};
	glm::mat4 projectionMatrix{};
	glm::vec3 position{};
	glm::vec3 targetPosition{};

	glm::vec3 frontVector{ 0.0f, 0.0f, -1.0f };
	glm::vec3 upVector{ 0.0f, 1.0f, 0.0f };
	glm::vec3 rightVector{ 1.0f, 0.0f, 0.0f };
};
