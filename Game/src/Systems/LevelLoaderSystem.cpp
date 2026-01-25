#include "LevelLoaderSystem.h"
#include "../Components/Physics.h"
#include "../Components/Render.h"
#include "../Components/Player.h"
#include "../Components/Camera.h"

#include "../ECSController.h"

extern ECSController controller;

Frustum CreateFrustum(float zFar, float zNear, float fovY, float aspectRatio, glm::vec3 front, glm::vec3 right, glm::vec3 up, glm::vec3 pos) {
	Frustum frust;
	const float halfVSide = zFar * tanf(fovY * 0.5f);
	const float halfHSide = halfVSide * aspectRatio;
	const glm::vec3 frontMultFar = zFar * front;

	frust.nearPlane = { pos + zNear * front, front };
	frust.farPlane = { pos + frontMultFar, -front };
	frust.right = { pos, glm::normalize(glm::cross(frontMultFar - right * halfHSide, up)) };
	frust.left = { pos, glm::normalize(glm::cross(up, frontMultFar + right * halfHSide)) };
	frust.top = { pos, glm::normalize(glm::cross(right, frontMultFar - up * halfVSide)) };
	frust.bottom = { pos, glm::normalize(glm::cross(frontMultFar + up * halfVSide, right)) };

	return frust;
}

AABB generateAABB(Model& model)
{
	glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

	for (const Mesh& mesh : model.getMeshes())
	{
		for (const Vertex& vertex : mesh.getVertices())
		{
			minAABB.x = std::min(minAABB.x, vertex.position.x);
			minAABB.y = std::min(minAABB.y, vertex.position.y);
			minAABB.z = std::min(minAABB.z, vertex.position.z);

			maxAABB.x = std::max(maxAABB.x, vertex.position.x);
			maxAABB.y = std::max(maxAABB.y, vertex.position.y);
			maxAABB.z = std::max(maxAABB.z, vertex.position.z);
		}
	}
	return AABB(minAABB, maxAABB);
}

Sphere generateBoundingSphere(std::shared_ptr<Model> model)
{
	glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

	for (const Mesh& mesh : model->getMeshes())
	{
		for (const Vertex& vertex : mesh.getVertices())
		{
			minAABB.x = std::min(minAABB.x, vertex.position.x);
			minAABB.y = std::min(minAABB.y, vertex.position.y);
			minAABB.z = std::min(minAABB.z, vertex.position.z);

			maxAABB.x = std::max(maxAABB.x, vertex.position.x);
			maxAABB.y = std::max(maxAABB.y, vertex.position.y);
			maxAABB.z = std::max(maxAABB.z, vertex.position.z);
		}
	}

	return Sphere((maxAABB + minAABB) * 0.5f, glm::length(minAABB - maxAABB));
}

void LevelLoaderSystem::LoadLevel()
{
	// create scene entities
	Entity entity = controller.createEntity();
	auto loaded = LoadModel("assets/models/snowy_mountain_-_terrain/scene.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(500.0f) });
	controller.AddComponent(entity, StaticModel{ nullptr, loaded.first });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second });
	controller.AddComponent(entity, PhysicsBody{});

	entity = controller.createEntity();
	loaded = LoadModel("assets/models/rubix_2.0/scene.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(3.0f, -5.0f, 10.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f) });
	controller.AddComponent(entity, StaticModel{ nullptr, loaded.first });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second });
	controller.AddComponent(entity, PhysicsBody{});


	entity = controller.createEntity();
	loaded = LoadModel("assets/models/2003_peugeot_hoggar_concept/scene.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(0.0f, -5.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(40.0f) });
	controller.AddComponent(entity, VehicleBody{});
	controller.AddComponent(entity, VehicleCommands{});
	controller.AddComponent(entity, Render{ loaded.first, loaded.second });
	controller.AddComponent(entity, PhysicsBody{});
	controller.AssignTag(entity, "VehicleCommands");
}

std::pair<std::shared_ptr<Model>, std::shared_ptr<BoundingVolume>> LevelLoaderSystem::LoadModel(std::string path)
{
	// check if model is previously loaded
	if (models.find(path) == models.end())
	{
		Entity camera = controller.GetEntityByTag("Camera");
		auto& cameraComp = controller.GetComponent<ThirdPersonCamera>(camera);

		models[path] = std::make_shared<Model>(path); // load the model
		boundingVolumes[path] = std::make_shared<AABB>(generateAABB(*models[path])); // create bounding volume for the model
	}

	return { models[path], boundingVolumes[path] };
}