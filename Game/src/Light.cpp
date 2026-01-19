#include "Light.h"

Light::Light()
{
	position = { 0.0f, 300.0f, 300.0f };

	lightProjection = glm::ortho(-300.0f, 300.0f, -250.0f, 100.0f, near, far);
	lightView = glm::lookAt(position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;
}