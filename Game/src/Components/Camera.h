#pragma once

#include "glm/glm.hpp"

struct ThirdPersonCamera
{
	float radius;
	float heightOffset;
	float lerpSpeed;
	float horizontalLookSpeed;
	float verticalLookSpeed;

	float yaw;
	float pitch;

	float fov;
	float zNear;
	float zFar;
	unsigned int screenWidth;
	unsigned int screenHeight;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	
	glm::mat4 getProjectionMatrix() const {
		return glm::perspective(glm::radians(fov), screenWidth / (float)screenHeight, zNear, zFar);
	}
};

