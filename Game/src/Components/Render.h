#pragma once

#include "../Core/Model.h"
#include "../Core/BoundingVolumes.h"
#include <memory>

struct Render
{
	std::shared_ptr<Model> model;
	std::shared_ptr<AABB> boundingVolume;
	bool isInstanced = false;
};

