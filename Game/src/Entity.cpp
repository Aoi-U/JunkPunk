#include "Entity.h"

Entity::Entity(Model modelPtr, const glm::mat4& modelMatrix)
{
	model = std::make_shared<Model>(modelPtr);
	this->modelMatrix = modelMatrix;
}
