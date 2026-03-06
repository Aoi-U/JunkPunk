#include "LevelLoaderSystem.h"
#include "../Components/Physics.h"
#include "../Components/Render.h"
#include "../Components/Player.h"
#include "../Components/Camera.h"
#include "../Components/Obstacle.h"
#include "../Components/Transform.h"
#include "../Components/Particles.h"
#include "../Components/Powerup.h"
#include "../Components/AiDriver.h"
#include "AiSystem.h"



#include "../ECSController.h"

extern ECSController controller;

void LevelLoaderSystem::SetAiSystem(std::shared_ptr<AiSystem> ai)
{
	aiSystemPtr = ai;
}

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
	// create camera entity (KEEP THIS AT THE VERY TOP OR ELSE IT WILL CRASH) (i know whats causing it but i cant think of a better solution)
	float radius = 5.0f;
	float heightOffset = 1.5f;
	float lerpSpeed = 4.0f;
	float horizontalLookSpeed = 5.0f;
	float verticalLookSpeed = 1.5f;
	float yaw = 0.0f;
	float pitch = 00.0f;
	float fov = 45.0f;
	float zNear = 0.1f;
	float zFar = 800.0f;
	glm::vec3 cameraPos = glm::vec3(0.0f, -5.0f, 0.0f) + glm::vec3(0.0f, heightOffset, -radius);
	glm::mat4 viewMatrix = glm::lookAt(cameraPos, glm::vec3(0.0f, -5.0f, 0.0f) + glm::vec3(0.0f, heightOffset, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), 1280 / (float)720, zNear, zFar);

	Entity camera = controller.createEntity();
	controller.AddComponent(camera, ThirdPersonCamera{ radius, radius, heightOffset, lerpSpeed, horizontalLookSpeed, verticalLookSpeed, yaw, pitch, fov, zNear, zFar, 1280, 720, viewMatrix, projectionMatrix });
	controller.AddComponent(camera, Transform{ cameraPos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f) });
	controller.AssignTag(camera, "Camera");
	// end camera entity

	// create scene entities
	//entity = controller.createEntity();
	//auto loaded = LoadModel("assets/models/snowy_mountain_-_terrain/scene.gltf");
	//controller.AddComponent(entity, Transform{ glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(500.0f) });
	//controller.AddComponent(entity, StaticBody{ nullptr, loaded.first });
	//controller.AddComponent(entity, Render{ loaded.first, loaded.second });
	//controller.AddComponent(entity, PhysicsBody{});

	// create dumpster
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::pi<float>()/4.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	Entity entity = controller.createEntity();
	auto loaded = LoadModel("assets/models/dumpster/dumpster.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(0.0f, -50.0f, 50.0f), glm::quat_cast(rotation), glm::vec3(50.0f) });
	controller.AddComponent(entity, StaticBody{ nullptr, loaded.first });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second });
	controller.AddComponent(entity, PhysicsBody{});

	//for (int i = 0; i < 10; i++)
	//{
	//	entity = controller.createEntity();
	//	loaded = LoadModel("assets/models/rubix_2.0/scene.gltf");
	//	controller.AddComponent(entity, Transform{ glm::vec3(0.0f, -40.0f + i * 8.0f, 20.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.7f) });
	//	controller.AddComponent(entity, RigidBody{ nullptr, loaded.first, loaded.second, 50.0f, true, glm::vec3(0.0f), glm::vec3(0.0f) });
	//	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	//	controller.AddComponent(entity, PhysicsBody{});
	//	controller.AddComponent(entity, MovingObstacle{
	//		std::vector<glm::vec3>{
	//			{ -50.0f - i, -90.0f + i * 2 ,-20.0f + i },
	//			{ -50.0f - i, -90.0f + i * 2, -30.0f + i },
	//			{ -40.0f - i, -90.0f + i * 2, -30.0f + i },
	//			{ -40.0f - i, -90.0f + i * 2, -20.0f + i }
	//		},
	//		0.0f,
	//		0,
	//		5.0f
	//		});
	//}

	//for (int i = 0; i < 50; i++)
	//{
	//	entity = controller.createEntity();
	//	loaded = LoadModel("assets/models/rubix_2.0/scene.gltf");
	//	controller.AddComponent(entity, Transform{ glm::vec3(-60.0f, -90.0f + i * 10.0f, 20.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.7f) });
	//	controller.AddComponent(entity, RigidBody{ nullptr, loaded.first, loaded.second, 50.0f, true, glm::vec3(0.0f), glm::vec3(0.0f) });
	//	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	//	controller.AddComponent(entity, PhysicsBody{});
	//}


	entity = controller.createEntity();
	loaded = LoadModel("assets/models/2003_peugeot_hoggar_concept/scene.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-26.931, -94.595, -27.625), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(40.0f) });
	controller.AddComponent(entity, VehicleBody{});
	controller.AddComponent(entity, VehicleCommands{});
	controller.AddComponent(entity, PlayerController{ 1 });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second });
	controller.AddComponent(entity, PhysicsBody{});
	controller.AssignTag(entity, "VehicleCommands");
	auto& cameraComp = controller.GetComponent<ThirdPersonCamera>(camera); // set the camera's player entity to the vehicle
	cameraComp.playerEntity = entity;

	ParticleEmitter rightWheel = ParticleEmitter{};
	rightWheel.Init(20000);
	rightWheel.targetEntity = controller.GetEntityByTag("VehicleCommands");
	rightWheel.offset = glm::vec3(-2.0f, 0.0f, 0.0f);
	entity = controller.createEntity();
	controller.AddComponent(entity, ParticleEmitter{
		rightWheel
		});

	// AI Opponent vehicle
	entity = controller.createEntity();
	auto aiLoaded = LoadModel("assets/models/2003_peugeot_hoggar_concept/scene.gltf");
	glm::mat4 aiRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	controller.AddComponent(entity, Transform{ glm::vec3(-35.0f, -80.0f, -15.0f), glm::quat(aiRotation), glm::vec3(40.0f) });
	controller.AddComponent(entity, VehicleBody{});
	controller.AddComponent(entity, VehicleCommands{});
	controller.AddComponent(entity, Render{ aiLoaded.first, aiLoaded.second });
	controller.AddComponent(entity, PhysicsBody{});
	controller.AssignTag(entity, "AIVehicle");

	// Initial paramerters for the AI driver, these can be tweaked to change the difficulty of the AI
	AiDriver ai{};
	//ai.desiredSpeed = 2.0f;
	//ai.arrivalRadius = 2.0f;
	//ai.maxSteerRadians = 1.0f;
	//ai.throttleKp = 0.6f;
	controller.AddComponent(entity, ai);

	// test trigger box
	//entity = controller.createEntity();
	//controller.AddComponent(entity, PhysicsBody{});
	//controller.AddComponent(entity, Trigger{ nullptr, 10.0f, 2.0f, 10.0f });
	//controller.AddComponent(entity, Transform{
	//	glm::vec3(25.0f, 35.0f, 120.0f),
	//	glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
	//	glm::vec3(1.0f)
	//	});

	// finish line
	entity = controller.createEntity();
	loaded = LoadModel("assets/models/lightning_capsule/scene.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-80.0f, -93.0f, 19.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f) });
	controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 2.0f, 1.0f });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.AddComponent(entity, Powerup{
		1,
		false,
		5.0f,
		0.0f
		});
  entity = controller.createEntity();
  loaded = LoadModel("assets/models/finishline/finishline.gltf");
  //controller.AddComponent(entity, StaticBody{ nullptr, loaded.first });
  controller.AddComponent(entity, Render{ loaded.first, loaded.second });
  controller.AddComponent(entity, PhysicsBody{});
  controller.AddComponent(entity, Trigger{ nullptr, 0.5f, 4.0f, 10.0f });
  controller.AddComponent(entity, Transform{
    glm::vec3(25.0f, -3.5f, 120.0f),
    glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
    glm::vec3(5.0f, 5.0f, 5.0f)
    });
  controller.AssignTag(entity, "FinishLine");

	//entity = controller.createEntity();
	//controller.AddComponent(entity, PhysicsBody{});
	//controller.AddComponent(entity, Transform{ glm::vec3(-60.0f, -93.0f, 19.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f) });
	//controller.AddComponent(entity, CheckPoint{ glm::quat(1.0f, 0.0f, 0.0f, 0.0f) });
	//controller.AddComponent(entity, Trigger{ nullptr, 5.0f, 2.0f, 5.0f });

  
  // later inside LoadLevel(), replace the direct call with a safe call:
  std::vector<Waypoint> waypoints;
  if (aiSystemPtr) {
	  waypoints = aiSystemPtr->GetWaypoints();
  }
  else {
	  // fallback: leave empty or build defaults
  }

  for (const Waypoint& wp : waypoints) {
	  entity = controller.createEntity();
	  controller.AddComponent(entity, PhysicsBody{});
	  controller.AddComponent(entity, Transform{ wp.position, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f) });
	  controller.AddComponent(entity, CheckPoint{ glm::quat(1.0f, 0.0f, 0.0f, 0.0f) });
	  controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 4.0f, 1.0f });
  }
}

std::pair<std::shared_ptr<Model>, std::shared_ptr<AABB>> LevelLoaderSystem::LoadModel(std::string path)
{
	// check if model is previously loaded
	if (loadedModels.find(path) == loadedModels.end())
	{
		std::shared_ptr<Model> model = std::make_shared<Model>(path); // load the model
		std::shared_ptr<AABB> bv = std::make_shared<AABB>(generateAABB(*model)); // create bounding volumes for the model
		loadedModels[path] = { model, bv };
	}

	return loadedModels[path];
}