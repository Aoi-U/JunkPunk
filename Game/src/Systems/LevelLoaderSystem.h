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

class AiSystem;
class AiSystemDebug; // forward declaration
class NavMesh;

class LevelLoaderSystem : public System
{
public:
	void LoadLevel();
	void SetAiSystem(std::shared_ptr<AiSystem> ai);

	// returns the model and its bounding volume for the render component
	std::pair<std::shared_ptr<Model>, std::shared_ptr<AABB>> LoadModel(std::string path);

	// Spawns banana peels at random positions along the NavMesh
	void SpawnRandomBananaPeels(int count, const NavMesh& navMesh);

	// Spawns powerups of a specific type at random positions along the NavMesh
	// powerupType: 1 = Speed Boost (lightning), 2 = Banana Pickup
	void SpawnRandomPowerups(int count, int powerupType, const NavMesh& navMesh);

	// Spawns a mix of both powerup types randomly across the NavMesh
	void SpawnRandomMixedPowerups(int speedBoostCount, int bananaCount, const NavMesh& navMesh);
private:

	// maps file name to its corresponding model and its bounding volume
	std::unordered_map<std::string, std::pair<std::shared_ptr<Model>, std::shared_ptr<AABB>>> loadedModels;
	std::shared_ptr<AiSystem> aiSystemPtr;
};