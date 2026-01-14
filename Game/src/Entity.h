#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "Model.h"

// entity class to represent objects in the game 
class Entity
{
public:
	Entity() = default;
	Entity(Model modelPtr, const glm::mat4& modelMatrix);

	std::shared_ptr<Model> getModel() { return model; }
	const glm::mat4& getModelMatrix() { return modelMatrix; }

	void setModelMatrix(const glm::mat4& matrix) { modelMatrix = matrix; }

	void Cleanup();

private:
	std::shared_ptr<Model> model;
	glm::mat4 modelMatrix{};
};