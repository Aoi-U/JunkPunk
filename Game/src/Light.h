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
	const glm::vec3& getDirection() { return glm::normalize(position); }

private:
	glm::vec3 position;

	glm::vec3 ambient{ 0.3f, 0.3f, 0.3f };
	glm::vec3 diffuse{ 0.8f, 0.8f, 0.8f };
	glm::vec3 specular{ 0.2f, 0.2f, 0.2f };
};