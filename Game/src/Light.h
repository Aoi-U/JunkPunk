#pragma once

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

class Light
{
public:
	Light();

	const glm::vec3& getPosition() const { return position; }

	const glm::vec3& getAmbient() const { return ambient; }
	const glm::vec3& getDiffuse() const { return diffuse; }
	const glm::vec3& getSpecular() const { return specular; }
	const glm::mat4& getLightSpaceMatrix() const { return lightSpaceMatrix; }

private:
	glm::vec3 position{ 100.0f, 200.0f, 200.0f };

	glm::vec3 ambient{ 0.7f, 0.7f, 0.7f };
	glm::vec3 diffuse{ 0.7f, 0.7f, 0.7f };
	glm::vec3 specular{ 1.0f, 1.0f, 1.0f };

	float near = 1.0f;
	float far = 500.0f;
	glm::mat4 lightProjection;
	glm::mat4 lightView;
	glm::mat4 lightSpaceMatrix;
};