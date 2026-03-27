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
#include"../Components/Banana.h"
#include "../Components/Sludge.h"


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
	float fov = 60.0f;
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
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
	Entity entity = controller.createEntity();
	auto loaded = LoadModel("assets/models/dumpster/dumpster.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(0.0f, -100.0f, 50.0f), glm::quat_cast(rotation), glm::vec3(200.0f) });
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
	//		{ // path points
	//			{ -50.0f - i, -90.0f + i * 2 ,-20.0f + i },
	//			{ -50.0f - i, -90.0f + i * 2, -30.0f + i },
	//			{ -40.0f - i, -90.0f + i * 2, -30.0f + i },
	//			{ -40.0f - i, -90.0f + i * 2, -20.0f + i },
	//			{ -40.0f - i, -90.0f + i * 2, -20.0f + i }
	//		},
	//		{ // rotation points (must be empty or same size as path points)
	//			glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f)),
	//			glm::quat(glm::vec3(0.0f, glm::radians(180.0f), 0.0f)),
	//			glm::quat(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f)),
	//			glm::quat(glm::vec3(0.0f, glm::radians(180.0f), 0.0f)),
	//			glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f))
	//		},
	//		{
	//			// times to reach each point (if empty, movement will be based on speed, must be empty or same size as path points)
	//			1.0f,
	//			2.0f,
	//			3.0f,
	//			5.0f,
	//			5.0f
	//		},
	//		0.0f,
	//		0,
	//		5.0f,
	//		false
	//		});
	//}

	//punching glove
	std::vector<std::vector<glm::vec3>> glove_positions = {
		{glm::vec3(20.0f, -169.f, 40.f), glm::vec3(20.0f, -169.f, -45.f), glm::vec3(20.0f, -169.f, -45.f), glm::vec3(20.0f, -169.f, 40.f)},
		{glm::vec3(-15.0f, -169.f, 40.f), glm::vec3(-15.0f, -169.f, -45.f), glm::vec3(-15.0f, -169.f, -45.f), glm::vec3(-15.0f, -169.f, 40.f)},
		{glm::vec3(-50.0f, -169.f, 40.f), glm::vec3(-50.0f, -169.f, -45.f), glm::vec3(-50.0f, -169.f, -45.f), glm::vec3(-50.0f, -169.f, 40.f)},
		{glm::vec3(45.0f, -70.f, 440.f), glm::vec3(45.0f, -67.f, 301.f), glm::vec3(45.0f, -67.f, 301.f), glm::vec3(45.0f, -70.f, 440.f)},
	};
	std::vector<float> glove_size = {
		10.f,
		10.f,
		10.f,
		10.f,
	};
	std::vector<float> glove_speed = {
		1.5f,
		1.5f,
		1.5f,
		1.5f,
	};

	for (int i = 0; i < glove_positions.size(); i++) {
		//punching glove
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/spring_glove/spring_glove.gltf");
		rotation = glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 4.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		controller.AddComponent(entity, Transform{ glm::vec3(-50.0f, -69.0f, 50.0f), glm::quat_cast(rotation), glm::vec3(glove_size[i])});
		controller.AddComponent(entity, RigidBody{ nullptr, loaded.first, loaded.second, 50.0f, true, glm::vec3(0.0f), glm::vec3(0.0f) });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.AddComponent(entity, MovingObstacle{
			glove_positions[i],
			std::vector<glm::quat>{
				glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f))),
				glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f))),
				glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f))),
				glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f))),
			},
			std::vector <float>{
				2.0f, glove_speed[i], 0.5f, 0.5f
			},
			0.0f,
			1,
			0,
			false
			});

	}

	


	//spinner
	std::vector<glm::vec3> spinner_positions = {
		glm::vec3(0.0f, -81.f, 100.f),
		glm::vec3(0.0f, -81.f, 180.f),
		glm::vec3(40.0f, -81.f, 150.f),
		glm::vec3(40.0f, -80.5f, 150.f),
		glm::vec3(60.0f, -80.5f, 115.f),
		glm::vec3(70.0f, -80.f, 170.f),
		glm::vec3(70.0f, -80.f, 200.f),
		glm::vec3(40.0f, -80.f, 205.f),
		glm::vec3(-19.0f, -80.f, 210.f),

		glm::vec3(37.f, -32.f, 295.f),
		glm::vec3(37.f, -34.f, 295.f),

		//glm::vec3(-56.f, 53.f, 358.f),
		//glm::vec3(-56.0f, 53.f, 293.f),
	};
	std::vector<float> spinner_rotation = {
		-1.f,
		1.f,
		1.f,
		1.f,
		1.f,
		-1.f,
		1.f,
		-1.f,
		1.f,

		1.f,
		-1.f,

		//1.f,
		//-1.f
	};
	std::vector<float> spinner_duration = {
		2.f,
		1.f,
		1.f,
		1.f,
		1.f,
		1.f,
		1.f,
		1.f,
		1.f,

		6.f,
		6.f,

		//1.2f,
		//1.2f
	};
	std::vector<float> spinner_size = {
		4.f,
		2.f,
		2.f,
		1.f,
		1.f,
		1.f,
		1.f,
		1.f,
		1.f,

		4.f,
		4.f,

		//3.f,
		//3.f
	};

	for (int i = 0; i < spinner_positions.size(); i++) {
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/spinner/spinner.gltf");
		rotation = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		controller.AddComponent(entity, Transform{ spinner_positions[i], glm::quat_cast(rotation), glm::vec3(spinner_size[i])});
		controller.AddComponent(entity, RigidBody{ nullptr, loaded.first, loaded.second, 50.0f, true, glm::vec3(0.0f), glm::vec3(0.0f) });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.AddComponent(entity, MovingObstacle{
			std::vector<glm::vec3>{
				spinner_positions[i],
				spinner_positions[i],
				spinner_positions[i],
			},
			std::vector<glm::quat>{
				glm::quat_cast(glm::rotate(glm::mat4(1.0f), -glm::pi<float>() * spinner_rotation[i], glm::vec3(0.0f, 1.0f, 0.0f))),
				glm::quat_cast(glm::rotate(glm::mat4(1.0f), 0.f, glm::vec3(0.0f, 1.0f, 0.0f))),
				glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::pi<float>() * spinner_rotation[i], glm::vec3(0.0f, 1.0f, 0.0f))),
			},
			std::vector <float>{
				0.f,spinner_duration[i],spinner_duration[i]
			},
			0.0f,
			1,
			0,
			false
			});
	}
	//90 degree offset
	entity = controller.createEntity();
	loaded = LoadModel("assets/models/spinner/spinner.gltf");
	rotation = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	controller.AddComponent(entity, Transform{glm::vec3(37.f, -32.f, 295.f), glm::quat_cast(rotation), glm::vec3(4.0f)});
	controller.AddComponent(entity, RigidBody{ nullptr, loaded.first, loaded.second, 50.0f, true, glm::vec3(0.0f), glm::vec3(0.0f) });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.AddComponent(entity, MovingObstacle{
		std::vector<glm::vec3>{
			glm::vec3(37.f, -32.f, 295.f),
			glm::vec3(37.f, -32.f, 295.f),
			glm::vec3(37.f, -32.f, 295.f),
		},
		std::vector<glm::quat>{
			glm::quat_cast(glm::rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f))),
			glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f))),
			glm::quat_cast(glm::rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f))),
		},
		std::vector <float>{
			0.f,6.f,0.f
		},
		0.0f,
		1,
		0,
		false
		});



	//dice
	std::vector<std::vector<glm::vec3>> dice_positions = {
		{glm::vec3(40.0f, -46.f, 200.f), glm::vec3(140.0f, -41.f, 200.f), glm::vec3(140.0f, -41.f, 200.f), glm::vec3(40.0f, -46.f, 200.f)},
		{glm::vec3(40.0f, -46.f, 175.f), glm::vec3(140.0f, -41.f, 175.f), glm::vec3(140.0f, -41.f, 175.f), glm::vec3(40.0f, -46.f, 175.f)},
		{glm::vec3(40.0f, -46.f, 150.f), glm::vec3(140.0f, -41.f, 150.f), glm::vec3(140.0f, -41.f, 150.f), glm::vec3(40.0f, -46.f, 150.f)},

		{glm::vec3(20.0f, -46.f, 200.f), glm::vec3(-40.0f, -46.f, 200.f), glm::vec3(-40.0f, -46.f, 200.f), glm::vec3(20.0f, -46.f, 200.f)},
		{glm::vec3(20.0f, -46.f, 175.f), glm::vec3(-40.0f, -46.f, 175.f), glm::vec3(-40.0f, -46.f, 175.f), glm::vec3(20.0f, -46.f, 175.f)},
		{glm::vec3(20.0f, -46.f, 150.f), glm::vec3(-40.0f, -46.f, 150.f), glm::vec3(-40.0f, -46.f, 150.f), glm::vec3(20.0f, -46.f, 150.f)},

		//{glm::vec3(0.0f, 62.f, 325.f), glm::vec3(30.0f, 62.f, 355.f), glm::vec3(60.0f, 62.f, 325.f), glm::vec3(30.0f, 62.f, 295.f)},
	};

	std::vector<std::vector<float>> dice_duration = {
		{3.f,5.f,3.f, 5.f},
		{3.f,7.f,3.f, 7.f},
		{3.f,9.f,3.f, 9.f},

		{3.f,5.f,3.f, 5.f},
		{3.f,7.f,3.f, 7.f},
		{3.f,9.f,3.f, 9.f},

		//{1.f,1.f, 1.f, 1.f},
	};
	std::vector<float> dice_size = {
		10.f,
		10.f,
		10.f,

		10.f,
		10.f,
		10.f,

		//10.f,
	};
	//moving dice
	for (int i = 0; i < dice_positions.size(); i++) {
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/dice/dice.gltf");
		rotation = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		controller.AddComponent(entity, Transform{ glm::vec3(0.f, 0.f, 0.f), glm::quat_cast(rotation), glm::vec3(dice_size[i])});
		controller.AddComponent(entity, RigidBody{ nullptr, loaded.first, loaded.second, 50.0f, true, glm::vec3(0.0f), glm::vec3(0.0f) });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.AddComponent(entity, MovingObstacle{
			dice_positions[i],
			std::vector<glm::quat>{},
			dice_duration[i],
			0.0f,
			1,
			0,
			false
			});
	}
	//for (int i = 0; i < 15; i++) {
	//	entity = controller.createEntity();
	//	loaded = LoadModel("assets/models/dice/dice.gltf");
	//	rotation = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	//	controller.AddComponent(entity, Transform{ glm::vec3(0.f, 0.f, 0.f), glm::quat_cast(rotation), glm::vec3(1.f) });
	//	controller.AddComponent(entity, RigidBody{ nullptr, loaded.first, loaded.second, 50.0f, true, glm::vec3(0.0f), glm::vec3(0.0f) });
	//	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	//	controller.AddComponent(entity, PhysicsBody{});
	//	controller.AddComponent(entity, MovingObstacle{
	//		std::vector<glm::vec3>{
	//			glm::vec3(65.0f + (i * 3), 54.5f, 410.f),
	//			glm::vec3(65.0f + (i * 3), 54.5f, 250.f),
	//			//glm::vec3(65.0f + (i * 3), 54.5f, 250.f),
	//			//glm::vec3(65.0f + (i * 3), 54.5f, 410.f)
	//		},
	//		std::vector<glm::quat>{},
	//		std::vector <float>{
	//			((rand() % 101) / 100.f) + 1.f,
	//			((rand() % 101) / 100.f) + 1.f,
	//			//((rand() % 101) / 100.f) + 1.f,
	//			//((rand() % 101) / 100.f) + 1.f
	//		}, 
	//		0.0f,
	//		1,
	//		0,
	//		false
	//		});
	//}
	

	for (int i = 0; i < 100; i++)
	{
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/dice/dice.gltf");
		controller.AddComponent(entity, Transform{ glm::vec3(0.0f, -247.0f + (i * 10.0f),-208.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.f) });
		controller.AddComponent(entity, RigidBody{ nullptr, loaded.first, loaded.second, 50.0f, true, glm::vec3(0.0f), glm::vec3(0.0f) });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
	}


	Entity vehicle = controller.createEntity();
	glm::mat4 player_rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	loaded = LoadModel("assets/models/car_body_orange/car.gltf");
	//controller.AddComponent(vehicle, Transform{ glm::vec3(-66.0f, 93.f, 323.f), glm::quat(player_rotation), glm::vec3(0.2f) });
	//controller.AddComponent(vehicle, Transform{ glm::vec3(148.0f, -26.f, 400.f), glm::quat(player_rotation), glm::vec3(0.2f) });
	controller.AddComponent(vehicle, Transform{ glm::vec3(137.0f, -255.0f, -233.f), glm::quat(player_rotation), glm::vec3(0.2f) });
	controller.AddComponent(vehicle, VehicleBody{});
	controller.AddComponent(vehicle, VehicleCommands{});
	controller.AddComponent(vehicle, PlayerController{ 1 });
	controller.AddComponent(vehicle, Render{ loaded.first, loaded.second });
	controller.AddComponent(vehicle, PhysicsBody{});
	controller.AssignTag(vehicle, "VehicleCommands");
	auto& cameraComp = controller.GetComponent<ThirdPersonCamera>(camera); // set the camera's player entity to the vehicle
	cameraComp.playerEntity = vehicle;

	entity = controller.createEntity(); // front left wheel
	loaded = LoadModel("assets/models/left_wheel/wheel.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-35.0f, -80.0f, -25.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.2f) });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

	entity = controller.createEntity(); // front right wheel
	loaded = LoadModel("assets/models/right_wheel/wheel.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-35.0f, -80.0f, -25.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.2f) });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

	entity = controller.createEntity(); // back left wheel
	loaded = LoadModel("assets/models/left_wheel/wheel.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-35.0f, -80.0f, -25.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.2f) });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

	entity = controller.createEntity(); // back right wheel
	loaded = LoadModel("assets/models/right_wheel/wheel.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-35.0f, -80.0f, -25.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.2f) });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

	ParticleEmitter particles = ParticleEmitter{};
	particles.Init(20000);
	particles.targetEntity = controller.GetEntityByTag("VehicleCommands");
	particles.offset = glm::vec3(-0.3f, 0.0f, -1.3f);
	entity = controller.createEntity();
	controller.AddComponent(entity, ParticleEmitter{
		particles
		});

	particles = ParticleEmitter{};
	particles.Init(20000);
	particles.targetEntity = controller.GetEntityByTag("VehicleCommands");
	particles.offset = glm::vec3(0.3f, 0.0f, -1.3f);
	entity = controller.createEntity();
	controller.AddComponent(entity, ParticleEmitter{
		particles
		});

	// AI Opponent vehicle
	vehicle = controller.createEntity();
	loaded = LoadModel("assets/models/car_body_blue/car.gltf");
	glm::mat4 aiRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	controller.AddComponent(vehicle, Transform{ glm::vec3(-35.0f, -80.0f, -15.0f), glm::quat(aiRotation), glm::vec3(0.2f) });
	controller.AddComponent(vehicle, VehicleBody{});
	controller.AddComponent(vehicle, VehicleCommands{});
	controller.AddComponent(vehicle, Render{ loaded.first, loaded.second });
	controller.AddComponent(vehicle, PhysicsBody{});
	controller.AssignTag(vehicle, "AIVehicle");

	entity = controller.createEntity(); // front left wheel
	loaded = LoadModel("assets/models/left_wheel/wheel.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-35.0f, -80.0f, -15.0f), glm::quat(aiRotation), glm::vec3(0.2f) });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

	entity = controller.createEntity(); // front right wheel
	loaded = LoadModel("assets/models/right_wheel/wheel.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-35.0f, -80.0f, -15.0f), glm::quat(aiRotation), glm::vec3(0.2f) });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

	entity = controller.createEntity(); // back left wheel
	loaded = LoadModel("assets/models/left_wheel/wheel.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-35.0f, -80.0f, -15.0f), glm::quat(aiRotation), glm::vec3(0.2f) });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

	entity = controller.createEntity(); // back right wheel
	loaded = LoadModel("assets/models/right_wheel/wheel.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-35.0f, -80.0f, -15.0f), glm::quat(aiRotation), glm::vec3(0.2f) });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

	// Initial paramerters for the AI driver, these can be tweaked to change the difficulty of the AI
	AiDriver ai{};
	//ai.desiredSpeed = 2.0f;
	//ai.arrivalRadius = 2.0f;
	//ai.maxSteerRadians = 1.0f;
	//ai.throttleKp = 0.6f;
	controller.AddComponent(vehicle, ai);

	// test trigger box
	//entity = controller.createEntity();
	//controller.AddComponent(entity, PhysicsBody{});
	//controller.AddComponent(entity, Trigger{ nullptr, 10.0f, 2.0f, 10.0f });
	//controller.AddComponent(entity, Transform{
	//	glm::vec3(25.0f, 35.0f, 120.0f),
	//	glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
	//	glm::vec3(1.0f)
	//	});

	entity = controller.createEntity();
	loaded = LoadModel("assets/models/lightning_capsule/scene.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(81.0f, -260.0f, -214.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f) });
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
	loaded = LoadModel("assets/models/banana/scene.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(81.0f, -260.0f, -254.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f) });
	controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 2.0f, 1.0f });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.AddComponent(entity, Powerup{
		2,
		false,
		5.0f,
		0.0f
		});

	entity = controller.createEntity();
	loaded = LoadModel("assets/models/bomb/scene.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(81.0f, -260.0f, -234.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(2.0f) });
	controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 2.0f, 1.0f });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.AddComponent(entity, Powerup{
		3,
		false,
		5.0f,
		0.0f
		});


	entity = controller.createEntity();
	loaded = LoadModel("assets/models/banana_peel/banana.gltf");
	controller.AddComponent(entity, Transform{ glm::vec3(-62.0f, -94.0f, -7.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.5f) });
	controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 1.0f, 1.0f });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.AddComponent(entity, Banana{});

	// finish line
  entity = controller.createEntity();
  loaded = LoadModel("assets/models/finishline/finishline.gltf");
  //controller.AddComponent(entity, StaticBody{ nullptr, loaded.first });
  controller.AddComponent(entity, Render{ loaded.first, loaded.second });
  controller.AddComponent(entity, PhysicsBody{});
  controller.AddComponent(entity, Trigger{ nullptr, 2.5f, 20.0f, 50.0f });
  controller.AddComponent(entity, Transform{
    glm::vec3(-184.0f, 115.f, 323.0f),
    glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
    glm::vec3(25.0f, 25.0f, 25.0f)
    });
  controller.AssignTag(entity, "FinishLine");

	entity = controller.createEntity();
	controller.AddComponent(entity, PhysicsBody{});
	controller.AddComponent(entity, Transform{ glm::vec3(-60.0f, -93.0f, 19.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f) });
	controller.AddComponent(entity, CheckPoint{ glm::quat(1.0f, 0.0f, 0.0f, 0.0f) });
	controller.AddComponent(entity, Trigger{ nullptr, 5.0f, 2.0f, 5.0f });

	// Used strictly for testing AI waypoints, can be removed later
	//std::vector<Waypoint> waypoints;
	//if (aiSystemPtr) {
	//	waypoints = aiSystemPtr->GetWaypoints();
	//	std::cout << "Loaded " << waypoints.size() << " waypoints for AI from AiSystem" << std::endl;
	//}
	//else {
	//	// fallback: leave empty or build defaults
	//	std::cout << "Warning: AiSystem not set in LevelLoaderSystem, no waypoints loaded for AI" << std::endl;
	//}
	//
	//for (const Waypoint& wp : waypoints) {
	//	entity = controller.createEntity();
	//	controller.AddComponent(entity, PhysicsBody{});
	//	controller.AddComponent(entity, Transform{ wp.position, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f) });
	//	controller.AddComponent(entity, CheckPoint{ glm::quat(1.0f, 0.0f, 0.0f, 0.0f) });
	//	controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 4.0f, 1.0f });
	//}

	std::vector<glm::vec3> sludge_positions = {
		glm::vec3(81.0f, -260.25f, -234.0f), // POSITION (adjust as needed)
		glm::vec3(-68.0f, -28.f, 258.7f),
		glm::vec3(-140.0f, -28.f, 315.f),
		glm::vec3(2.6f, -28.f, 392.5),
		glm::vec3(150.f, -28.f, 342.5),

		glm::vec3(90.f, -12.f, 412.5),
		glm::vec3(-45.f, -28.f, 382.5),
	};
	std::vector<glm::mat4> sludge_rotation = {
		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),

		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
	};
	std::vector<glm::vec3> sludge_scale = {
		glm::vec3(20.0f, 16.f, 20.f),
		glm::vec3(20.0f, 16.f, 40.f),
		glm::vec3(20.0f, 16.f, 40.f),
		glm::vec3(80.0f, 16.f, 20.f),
		glm::vec3(20.0f, 16.f, 40.f),

		glm::vec3(40.0f, 32.f, 20.f),
		glm::vec3(40.0f, 32.f, 20.f),
	};
	std::vector<glm::vec3> sludge_trigger_size = {
		glm::vec3(20.0f, 0.5f, 20.f),
		glm::vec3(20.0f, 0.5f, 40.f),
		glm::vec3(20.0f, 0.5f, 40.f),
		glm::vec3(80.0f, 0.5f, 20.f),
		glm::vec3(20.0f, 0.5f, 40.f),

		glm::vec3(40.0f, 1.f, 20.f),
		glm::vec3(40.0f, 1.f, 20.f),
	};

	for (int i = 0; i < sludge_positions.size(); i++) {
		entity = controller.createEntity();
		Sludge sludge;
		loaded = LoadModel("assets/models/sludge/sludge.gltf");
		controller.AddComponent(entity, Render{ loaded.first, loaded.second });
		controller.AddComponent(entity, Transform{
			sludge_positions[i],
			glm::quat(sludge_rotation[i]),
			sludge_scale[i],
			});

		controller.AddComponent(entity, Trigger{
			nullptr,
			sludge_trigger_size[i].x, // width
			sludge_trigger_size[i].y,  // height
			sludge_trigger_size[i].z  // length
			});

		controller.AddComponent(entity, Sludge{
			sludge.slowFactor // slow factor
			});

		controller.AddComponent(entity, PhysicsBody{});
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