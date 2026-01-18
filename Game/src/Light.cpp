#include "Light.h"

Light::Light()
{
	lightProjection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, near, far);
	lightView = glm::lookAt(position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;
}