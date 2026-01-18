#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

class Light
{
public:
	Light() = default;

	const glm::vec3& getPosition() const { return position; }

	const glm::vec3& getAmbient() const { return ambient; }
	const glm::vec3& getDiffuse() const { return diffuse; }
	const glm::vec3& getSpecular() const { return specular; }

private:
	glm::vec3 position{ 500.0f, 50.0f, 650.0f };

	glm::vec3 ambient{ 0.4f, 0.4f, 0.4f };
	glm::vec3 diffuse{ 0.5f, 0.5f, 0.5f };
	glm::vec3 specular{ 1.0f, 1.0f, 1.0f };
};