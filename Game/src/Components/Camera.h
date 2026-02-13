#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Core/Types.h"

struct ThirdPersonCamera
{
	float baseRadius;
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

	Entity playerEntity = MAX_ENTITIES; // entity that the camera will follow
	
	glm::mat4 getProjectionMatrix() const {
		return glm::perspective(glm::radians(fov), screenWidth / (float)screenHeight, zNear, zFar);
	}
};

