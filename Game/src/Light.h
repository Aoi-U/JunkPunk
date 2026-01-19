#pragma once

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

class Light
{
public:
	Light();

	const glm::vec3& getPosition() { return position; }

	const glm::vec3& getAmbient() { return ambient; }
	const glm::vec3& getDiffuse() { return diffuse; }
	const glm::vec3& getSpecular() { return specular; }
	const glm::mat4& getLightSpaceMatrix() { return lightSpaceMatrix; }

private:
	glm::vec3 position;

	glm::vec3 ambient{ 0.3f, 0.3f, 0.3f };
	glm::vec3 diffuse{ 0.8f, 0.8f, 0.8f };
	glm::vec3 specular{ 0.2f, 0.2f, 0.2f };

	float near = 1.0f;
	float far = 700.0f;
	glm::mat4 lightProjection;
	glm::mat4 lightView;
	glm::mat4 lightSpaceMatrix;
};