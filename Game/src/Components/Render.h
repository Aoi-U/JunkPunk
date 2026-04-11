#pragma once

#include "../Core/Model.h"
#include "../Core/BoundingVolumes.h"
#include <memory>
#include <glm/glm.hpp>

struct Render
{
	std::shared_ptr<Model> model;
	std::shared_ptr<AABB> boundingVolume;
	bool isInstanced = false;

	bool useFlatColor = false;
	glm::vec3 flatColor = glm::vec3(1.0f, 1.0f, 1.0f);
};

