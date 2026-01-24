#include "LevelLoaderSystem.h"
#include "../Components/Physics.h"
#include "../Components/Render.h"
#include "../Components/Transform.h"
#include "../Components/Player.h"

#include "../ECSController.h"

extern ECSController controller;

void LevelLoaderSystem::LoadLevel()
{
	// create scene entities
	auto loaded = LoadModel("assets/models/snowy_mountain_-_terrain/scene.gltf");
	Entity entity = loaded.first;
	controller.AddComponent(entity, Transform{ glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(500.0f) });
	controller.AddComponent(entity, StaticModel{ nullptr, loaded.second });
	controller.AddComponent(entity, PhysicsBody{});


	loaded = LoadModel("assets/models/rubix_2.0/scene.gltf");
	entity = loaded.first;
	controller.AddComponent(entity, Transform{ glm::vec3(3.0f, -5.0f, 10.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f) });
	controller.AddComponent(entity, StaticModel{ nullptr, loaded.second });
	controller.AddComponent(entity, PhysicsBody{});


	loaded = LoadModel("assets/models/2003_peugeot_hoggar_concept/scene.gltf");
	entity = loaded.first;
	controller.AddComponent(entity, Transform{ glm::vec3(0.0f, -5.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(40.0f) });
	controller.AddComponent(entity, VehicleBody{});
	controller.AddComponent(entity, Player{});
	controller.AddComponent(entity, PhysicsBody{});
	controller.AssignTag(entity, "Player");
}

std::pair<Entity, std::shared_ptr<Model>> LevelLoaderSystem::LoadModel(std::string path)
{
	Entity entity = controller.createEntity();
	// check if model is loaded
	if (models.find(path) == models.end())
	{
		// cut out only the file name from the 
		models[path] = std::make_shared<Model>(path);
	}

	controller.AddComponent(entity, Render{ models[path] });
	return { entity, models[path] };
}