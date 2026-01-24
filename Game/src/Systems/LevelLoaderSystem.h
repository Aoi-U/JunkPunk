#pragma once

#include <memory>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../Core/Model.h"
#include "../Systems/System.h"
#include "../Core/Types.h"

class LevelLoaderSystem : public System
{
public:
	void LoadLevel();

private:
	std::pair<Entity, std::shared_ptr<Model>> LoadModel(std::string path);


	std::unordered_map<std::string, std::shared_ptr<Model>> models;
};