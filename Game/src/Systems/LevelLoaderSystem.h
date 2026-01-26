#pragma once

#include <memory>
#include <array>
#include <unordered_map>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../Core/BoundingVolumes.h"
#include "../Core/Model.h"
#include "../Core/Types.h"
#include "../Systems/System.h"



class LevelLoaderSystem : public System
{
public:
	void LoadLevel();

	// returns the model and its bounding volume for the render component
	std::pair<std::shared_ptr<Model>, std::shared_ptr<AABB>> LoadModel(std::string path);
private:

	std::unordered_map<std::string, std::shared_ptr<AABB>> boundingVolumes; // maps file name to bounding volume
	std::unordered_map<std::string, std::shared_ptr<Model>> models; // maps file name to model
};