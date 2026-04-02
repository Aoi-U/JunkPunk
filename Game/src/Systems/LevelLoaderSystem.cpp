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
#include "../NavMesh.h"
#include "../Components/DangerZone.h"
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

	//Entity camera = controller.createEntity();
	//controller.AddComponent(camera, ThirdPersonCamera{ radius, radius, heightOffset, lerpSpeed, horizontalLookSpeed, //verticalLookSpeed, yaw, pitch, fov, zNear, zFar, 1280, 720, viewMatrix, projectionMatrix });
	//controller.AddComponent(camera, Transform{ cameraPos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f) });
	//controller.AssignTag(camera, "Camera");

	for (int i = 0; i < numPlayers; i++)
	{
		Entity camera = controller.createEntity();
		controller.AddComponent(camera, ThirdPersonCamera{ radius, radius, heightOffset, lerpSpeed, horizontalLookSpeed, verticalLookSpeed, yaw, pitch, fov, zNear, zFar, 1280, 720, viewMatrix, projectionMatrix });
		controller.AddComponent(camera, Transform{ cameraPos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f) });
		controller.AssignTag(camera, "Camera" + std::to_string(i + 1));

		cameraEntities.push_back(camera);
	}

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

	// Update this when moving to new map
	if (aiSystemPtr && numAi > 0)
	{
		NavMesh navMesh;
		navMesh.BuildFromModel(
			loaded.first,
			glm::vec3(0.0f, -100.0f, 50.0f),          // same position as the dumpster
			glm::quat_cast(rotation),                   // same rotation
			glm::vec3(200.0f),                            // same scale
			35.0f                                        // max slope angle – tweak if too many/few triangles
		);
		navMesh.Subdivide();    // 1023 -> 4092 triangles (4x denser)
		//navMesh.Subdivide(); // uncomment for 16368 triangles (16x denser) if needed
		navMesh.BuildAdjacency();
		navMesh.StitchDisconnectedIslands(30.0f, 3.0f);  // bridge gaps: 30 units horizontal, 15 units vertical max
		navMesh.ComputeEdgeDanger(3);  // spread danger 3 triangles inward from edges

		int32_t componentCount = navMesh.CountConnectedComponents();
		std::cout << "[LevelLoader] NavMesh has " << componentCount << " disconnected island(s) after stitching" << std::endl;

		aiSystemPtr->SetNavMesh(navMesh);

		// Spawn random banana peels along the track
		SpawnRandomBananaPeels(30, navMesh);  // Spawn 30 random banana peels

		// Spawn random powerups along the track
		//SpawnRandomMixedPowerups(10, 10, navMesh);  // 10 speed boosts, 10 banana pickups
	}

	//punching glove
	std::vector<std::vector<glm::vec3>> glove_positions = {
		{glm::vec3(20.0f, -169.f, 40.f), glm::vec3(20.0f, -169.f, -45.f), glm::vec3(20.0f, -169.f, -45.f), glm::vec3(20.0f, -169.f, 40.f)},
		{glm::vec3(-15.0f, -169.f, 40.f), glm::vec3(-15.0f, -169.f, -45.f), glm::vec3(-15.0f, -169.f, -45.f), glm::vec3(-15.0f, -169.f, 40.f)},
		{glm::vec3(-50.0f, -169.f, 40.f), glm::vec3(-50.0f, -169.f, -45.f), glm::vec3(-50.0f, -169.f, -45.f), glm::vec3(-50.0f, -169.f, 40.f)},
		{glm::vec3(45.0f, -70.f, 440.f), glm::vec3(45.0f, -67.f, 301.f), glm::vec3(45.0f, -67.f, 301.f), glm::vec3(45.0f, -70.f, 440.f)},
		{glm::vec3(-74.0f, 59.f, 500.f), glm::vec3(-74.0f, 64.f, 363.f), glm::vec3(-74.0f, 64.f, 363.f), glm::vec3(-74.0f, 59.f, 500.f)}, //top area
	};
	std::vector<float> glove_size = {
		10.f,
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
		2.f
	};

	for (int i = 0; i < glove_positions.size(); i++) {
		//punching glove
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/spring_glove/spring_glove.gltf");
		rotation = glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 4.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		controller.AddComponent(entity, Transform{ glove_positions[i][0], glm::quat_cast(rotation), glm::vec3(glove_size[i]) });
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

		Entity gloveEntity = entity; // save the glove entity we just created

		// Spawn danger zone covering the glove's full sweep path
		{
			// For punching gloves: only use the retracted and extended positions (indices 0 and 1)
			glm::vec3 retractedPos = glove_positions[i][0];  // e.g., (20, -169, 40)
			glm::vec3 extendedPos = glove_positions[i][1];   // e.g., (20, -169, -45)

			glm::vec3 extensionVector = extendedPos - retractedPos;
			float extensionLength = glm::length(extensionVector);
			glm::vec3 extensionDirection = glm::normalize(extensionVector);

			float SIDE_PADDING = 12.0f;      // width of the glove (X/Y perpendicular to extension)
			float EXTENSION_PADDING = 30.0f; // extra reach beyond extended position (for glove physical size + safety)
			float REAR_PADDING = 5.0f;       // small buffer behind retracted position

			glm::vec3 fullyExtendedPos = extendedPos + (extensionDirection * EXTENSION_PADDING);
			glm::vec3 paddedRetractedPos = retractedPos - (extensionDirection * REAR_PADDING);

			// Calculate AABB covering the full sweep
			glm::vec3 minPt = glm::min(paddedRetractedPos, fullyExtendedPos);
			glm::vec3 maxPt = glm::max(paddedRetractedPos, fullyExtendedPos);

			glm::vec3 center = (minPt + maxPt) * 0.5f;
			glm::vec3 halfExtents = (maxPt - minPt) * 0.5f;

			// Add side padding to X and Y dimensions only
			glm::vec3 finalHalfExtents = halfExtents;
			finalHalfExtents.x += SIDE_PADDING;
			finalHalfExtents.y += SIDE_PADDING;

			// Create the danger zone entity
			Entity dangerEntity = controller.createEntity();
			controller.AddComponent(dangerEntity, DangerZone{ center, finalHalfExtents, gloveEntity, SIDE_PADDING });
			controller.AddComponent(dangerEntity, PhysicsBody{});
			controller.AddComponent(dangerEntity, Transform{ center, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f) });
			controller.AddComponent(dangerEntity, Trigger{ nullptr, finalHalfExtents.x, finalHalfExtents.y, finalHalfExtents.z });

			std::cout << "[LevelLoader] Danger zone for glove " << i
				<< " spans from (" << minPt.x << ", " << minPt.y << ", " << minPt.z << ")"
				<< " to (" << maxPt.x << ", " << maxPt.y << ", " << maxPt.z << ")" << std::endl;
		}

	}

	//diagonal glove
	entity = controller.createEntity();
	loaded = LoadModel("assets/models/spring_glove/spring_glove.gltf");
	rotation = glm::rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.5f, glm::vec3(1.0f, 0.0f, 0.0f));
	controller.AddComponent(entity, Transform{ glm::vec3(-50.0f, -69.0f, 50.0f), glm::quat_cast(rotation), glm::vec3(10.f) });
	controller.AddComponent(entity, RigidBody{ nullptr, loaded.first, loaded.second, 50.0f, true, glm::vec3(0.0f), glm::vec3(0.0f) });
	controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
	controller.AddComponent(entity, PhysicsBody{});
	controller.AddComponent(entity, MovingObstacle{
		{glm::vec3(-4.0f, -232.f, 1.f), glm::vec3(-4.0f, -150.f, 11.f), glm::vec3(-4.0f, -150.f, 11.f), glm::vec3(-4.0f, -232.f, 1.f)},
		std::vector<glm::quat>{
			glm::quat_cast(rotation),
			glm::quat_cast(rotation),
			glm::quat_cast(rotation),
			glm::quat_cast(rotation),
		},
		std::vector <float>{
			2.0f, 2.f, 0.5f, 0.5f
		},
		0.0f,
		1,
		0,
		false
		});



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
		//top area
		glm::vec3(-39.f, 53.f, 330.f),
		glm::vec3(33.0f, 53.f, 305.f),
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

		1.f,
		-1.f
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

		1.2f,
		1.2f
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

		3.f,
		3.f
	};

	for (int i = 0; i < spinner_positions.size(); i++) {
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/spinner/spinner.gltf");
		rotation = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		controller.AddComponent(entity, Transform{ spinner_positions[i], glm::quat_cast(rotation), glm::vec3(spinner_size[i]) });
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
				glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::pi<float>()* spinner_rotation[i], glm::vec3(0.0f, 1.0f, 0.0f))),
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
	controller.AddComponent(entity, Transform{ glm::vec3(37.f, -32.f, 295.f), glm::quat_cast(rotation), glm::vec3(4.0f) });
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
		controller.AddComponent(entity, Transform{ glm::vec3(0.f, 0.f, 0.f), glm::quat_cast(rotation), glm::vec3(dice_size[i]) });
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

	//movable dice
	for (int i = 0; i < 100; i++)
	{
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/dice/dice.gltf");
		controller.AddComponent(entity, Transform{ glm::vec3(0.0f, -247.0f + (i * 10.0f),-248.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.f) });
		controller.AddComponent(entity, RigidBody{ nullptr, loaded.first, loaded.second, 50.0f, true, glm::vec3(0.0f), glm::vec3(0.0f) });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
	}

	for (int i = 0; i < numPlayers; i++)
	{
		Entity vehicle = controller.createEntity();
		glm::mat4 player_rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		loaded = LoadModel("assets/models/car_body_orange/car.gltf");
		glm::mat4 playerRotation = glm::rotate(glm::mat4(1.0f), glm::radians(38.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		Transform vehicleTransform{ glm::vec3(133.0f - i * 2.0f, -259.0f, -257.0f), glm::quat(playerRotation), glm::vec3(0.2f) };
		controller.AddComponent(vehicle, vehicleTransform);
		// controller.AddComponent(vehicle, Transform{ glm::vec3(148.0f, -26.f, 400.f), glm::quat(player_rotation), glm::vec3(0.2f) }); //testing tunnel
		// controller.AddComponent(vehicle, Transform{ glm::vec3(137.0f, -255.0f, -233.f), glm::quat(player_rotation), glm::vec3(0.2f) }); //beginning
		controller.AddComponent(vehicle, VehicleBody{});
		controller.AddComponent(vehicle, VehicleCommands{});
		controller.AddComponent(vehicle, PlayerController{ i + 1 });
		controller.AddComponent(vehicle, Render{ loaded.first, loaded.second });
		controller.AddComponent(vehicle, PhysicsBody{});
		controller.AssignTag(vehicle, "Player" + std::to_string(i + 1));
		auto& cameraComp = controller.GetComponent<ThirdPersonCamera>(controller.GetEntityByTag("Camera" + std::to_string(i + 1))); // set the camera's player entity to the vehicle
		cameraComp.playerEntity = vehicle;

		entity = controller.createEntity(); // front left wheel
		loaded = LoadModel("assets/models/left_wheel/wheel.gltf");
		controller.AddComponent(entity, vehicleTransform);
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

		entity = controller.createEntity(); // front right wheel
		loaded = LoadModel("assets/models/right_wheel/wheel.gltf");
		controller.AddComponent(entity, vehicleTransform);
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

		entity = controller.createEntity(); // back left wheel
		loaded = LoadModel("assets/models/left_wheel/wheel.gltf");
		controller.AddComponent(entity, vehicleTransform);
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

		entity = controller.createEntity(); // back right wheel
		loaded = LoadModel("assets/models/right_wheel/wheel.gltf");
		controller.AddComponent(entity, vehicleTransform);
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.GetComponent<VehicleBody>(vehicle).wheelEntities.push_back(entity);

		ParticleEmitter particles = ParticleEmitter{};
		particles.Init(20000);
		particles.targetEntity = controller.GetEntityByTag("Player" + std::to_string(i + 1));
		particles.offset = glm::vec3(-0.3f, 0.0f, -1.3f);
		entity = controller.createEntity();
		controller.AddComponent(entity, ParticleEmitter{
											particles });
		particles = ParticleEmitter{};
		particles.Init(20000);
		particles.targetEntity = controller.GetEntityByTag("Player" + std::to_string(i + 1));
		particles.offset = glm::vec3(0.3f, 0.0f, -1.3f);
		entity = controller.createEntity();
		controller.AddComponent(entity, ParticleEmitter{
											particles });

		particles = ParticleEmitter{};
		particles.Init(3000);
		particles.targetEntity = controller.GetEntityByTag("Player" + std::to_string(i + 1));
		particles.offset = glm::vec3(0.0f, 0.2f, 0.0f);
		particles.spawnRate = 0.0f;
		particles.life = 1.0f;
		particles.isBlastEmitter = true;
		particles.useBlastTexture = true;

		// set these to blast.png layout
		particles.atlasColumns = 9;
		particles.atlasRows = 8;
		particles.atlasFrameCount = 64;
		entity = controller.createEntity();
		controller.AddComponent(entity, ParticleEmitter{
											particles });
	}

	for (int i = 0; i < numAi; i++)
	{
		// AI Opponent vehicle
		Entity vehicle = controller.createEntity();
		loaded = LoadModel("assets/models/car_body_blue/car.gltf");
		glm::mat4 aiRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		controller.AddComponent(vehicle, Transform{ glm::vec3(133.0f + i * 2.0f, -259.0f, -257.0f), glm::quat(aiRotation), glm::vec3(0.2f) });
		controller.AddComponent(vehicle, VehicleBody{});
		controller.AddComponent(vehicle, VehicleCommands{});
		controller.AddComponent(vehicle, Render{ loaded.first, loaded.second });
		controller.AddComponent(vehicle, PhysicsBody{});
		controller.AssignTag(vehicle, "AIVehicle" + std::to_string(i));

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
		//AiDriver ai{};
		//ai.desiredSpeed = 2.0f;
		//ai.arrivalRadius = 2.0f;
		//ai.maxSteerRadians = 1.0f;
		//ai.throttleKp = 0.6f;
		controller.AddComponent(vehicle, AiDriver{});

		ParticleEmitter particles = ParticleEmitter{};
		particles.Init(10000);
		//particles.targetEntity = controller.GetEntityByTag("Player" + std::to_string(i + 1));
		particles.targetEntity = vehicle;
		particles.offset = glm::vec3(-0.3f, 0.0f, -1.3f);
		Entity particleEntity = controller.createEntity();
		controller.AddComponent(particleEntity, ParticleEmitter{
			particles
			});

		particles = ParticleEmitter{};
		particles.Init(10000);
		//particles.targetEntity = controller.GetEntityByTag("Player" + std::to_string(i + 1));
		particles.targetEntity = vehicle;
		particles.offset = glm::vec3(0.3f, 0.0f, -1.3f);
		particleEntity = controller.createEntity();
		controller.AddComponent(particleEntity, ParticleEmitter{
			particles
			});

		particles = ParticleEmitter{};
		particles.Init(3000);
		particles.targetEntity = vehicle;
		particles.offset = glm::vec3(0.0f, 0.2f, 0.0f);
		particles.spawnRate = 0.0f;
		particles.life = 1.0f;
		particles.isBlastEmitter = true;
		particles.useBlastTexture = true;

		// set these to blast.png layout
		particles.atlasColumns = 9;
		particles.atlasRows = 8;
		particles.atlasFrameCount = 64;
		particleEntity = controller.createEntity();
		controller.AddComponent(particleEntity, ParticleEmitter{
			particles
			});
	}

	// test trigger box
	//entity = controller.createEntity();
	//controller.AddComponent(entity, PhysicsBody{});
	//controller.AddComponent(entity, Trigger{ nullptr, 10.0f, 2.0f, 10.0f });
	//controller.AddComponent(entity, Transform{
	//	glm::vec3(25.0f, 35.0f, 120.0f),
	//	glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
	//	glm::vec3(1.0f)
	//	});

	std::vector<glm::vec3> boost_powerup_positions = {
		glm::vec3(81.0f, -259.5f, -214.0f), //starter
		glm::vec3(-139.0f, -135.f, 113.0f), //in the corner of the first tunnel
		glm::vec3(17.f, -138.f, -17.f), //behind the vertical puncher
		glm::vec3(45.f, -77.f, 290.f), //in the punching cave on the spinner area
		glm::vec3(150.f, -27.f, 289.f), //at the beginning of the sludge tunnel
		glm::vec3(-51.f, -8.f, 360.f), //at the end of the sludge tunnel
		glm::vec3(-161.f, 55.f, 296.f), //at the top ramp side
	};

	for (int i = 0; i < boost_powerup_positions.size(); i++) {
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/batterybox/battery_powerup.gltf");
		controller.AddComponent(entity, Transform{ boost_powerup_positions[i], glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.f) });
		controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 2.0f, 1.0f });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.AddComponent(entity, Powerup{
			1,
			false,
			5.0f,
			0.0f
			});
	}

	std::vector<glm::vec3> banana_powerup_positions = {
		glm::vec3(81.0f, -259.5f, -254.0f), //starter
		glm::vec3(-10.0f, -210.5f, -108.0f), //on the path
		glm::vec3(-3.f, -138.f, -33.f), //behind the vertical puncher
		glm::vec3(108.f, -78.f, 214.f), //in the left corner of the spinner area
		glm::vec3(160.f, -26.f, 289.f), //at the beginning of the sludge tunnel
		glm::vec3(-15.f, -26.f, 279.f), //at the end of the sludge tunnel
		glm::vec3(162.f, 55.f, 378.f), //at the top, back side
		glm::vec3(1.f, 55.f, 416.f), //at the top, front side
	};
	for (int i = 0; i < banana_powerup_positions.size(); i++) {
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/bananabox/banana_powerup.gltf");
		controller.AddComponent(entity, Transform{ banana_powerup_positions[i], glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f) });
		controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 2.0f, 1.0f });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.AddComponent(entity, Powerup{
			2,
			false,
			5.0f,
			0.0f
			});
	}


	std::vector<glm::vec3> bomb_powerup_positions = {
		glm::vec3(81.0f, -259.5f, -234.0f), //starter
		glm::vec3(-10.0f, -210.5f, -93.0f), //on the path
		glm::vec3(-20.f, -138.f, -15.f), //behind the vertical puncher
		glm::vec3(-81.f, -33.f, 96.f), //on the ledge before the dice crossing
		glm::vec3(140.f, -26.f, 289.f), //at the beginning of the sludge tunnel
		glm::vec3(-62.f, -26.f, 314.f), //at the end of the sludge tunnel
		glm::vec3(-104.f, 55.f, 322.f), //at the top
	};
	for (int i = 0; i < bomb_powerup_positions.size(); i++) {
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/bombbox/bomb_powerup.gltf");
		controller.AddComponent(entity, Transform{ bomb_powerup_positions[i], glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f) });
		controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 2.0f, 1.0f });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.AddComponent(entity, Powerup{
			3,
			false,
			5.0f,
			0.0f
			});
	}



	std::vector<glm::vec3> banana_peel_positions = {
		glm::vec3(-69.f, 56.f, 305.f), //at the top
		glm::vec3(-77.f, 56.f, 342.f), //at the top
		glm::vec3(-70.f, 56.f, 326.f), //at the top
	};
	for (int i = 0; i < banana_peel_positions.size(); i++) {
		entity = controller.createEntity();
		loaded = LoadModel("assets/models/banana_peel/banana.gltf");
		controller.AddComponent(entity, Transform{ banana_peel_positions[i], glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(2.f) });
		controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 1.0f, 1.0f });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.AddComponent(entity, Banana{});
	}


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

	std::vector<glm::vec3> sludge_positions = {
		//glm::vec3(81.0f, -260.25f, -234.0f), // POSITION (adjust as needed)
		glm::vec3(-83.0f, -28.f, 258.7f),
		glm::vec3(-140.0f, -28.f, 315.f),
		glm::vec3(2.6f, -28.f, 392.5),
		glm::vec3(150.f, -28.f, 342.5),

		glm::vec3(90.f, -12.f, 412.5),
		glm::vec3(-90.f, -12.f, 412.5),
		glm::vec3(-45.f, -28.f, 382.5),

		//at the top
		glm::vec3(95.0f, 55.f, 365.0f),
		glm::vec3(-52.0f, 56.f, 270.0f)
	};
	std::vector<glm::mat4> sludge_rotation = {
		//glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),

		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)),

		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
	};
	std::vector<glm::vec3> sludge_scale = {
		//glm::vec3(20.0f, 16.f, 20.f),
		glm::vec3(20.0f, 16.f, 40.f),
		glm::vec3(20.0f, 16.f, 40.f),
		glm::vec3(80.0f, 16.f, 20.f),
		glm::vec3(20.0f, 16.f, 40.f),

		glm::vec3(40.0f, 32.f, 20.f),
		glm::vec3(40.0f, 32.f, 20.f),
		glm::vec3(40.0f, 32.f, 20.f),

		glm::vec3(20.0f, 16.f, 20.f),
		glm::vec3(20.0f, 16.f, 20.f),
	};
	std::vector<glm::vec3> sludge_trigger_size = {
		//glm::vec3(20.0f, 0.5f, 20.f),
		glm::vec3(20.0f, 0.5f, 40.f),
		glm::vec3(20.0f, 0.5f, 40.f),
		glm::vec3(80.0f, 0.5f, 20.f),
		glm::vec3(20.0f, 0.5f, 40.f),

		glm::vec3(40.0f, 1.f, 20.f),
		glm::vec3(40.0f, 1.f, 20.f),
		glm::vec3(40.0f, 1.f, 20.f),

		glm::vec3(20.0f, 1.f, 20.f),
		glm::vec3(20.0f, 1.f, 20.f),
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

	// Used for testing A* pathfinding and debug rendering of AI waypoints
	//if (aiSystemPtr)
	//{
	//	Entity aiVehicle = controller.GetEntityByTag("AIVehicle1");
	//	aiSystemPtr->SpawnDebugWaypoints(aiVehicle);
	//}

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

void LevelLoaderSystem::SpawnRandomBananaPeels(int count, const NavMesh& navMesh)
{
	if (navMesh.IsEmpty())
	{
		std::cout << "[LevelLoader] Cannot spawn bananas: NavMesh is empty" << std::endl;
		return;
	}

	auto loaded = LoadModel("assets/models/banana_peel/banana.gltf");

	std::cout << "[LevelLoader] Spawning " << count << " random banana peels..." << std::endl;

	for (int i = 0; i < count; ++i)
	{
		// Get a random triangle from the navmesh
		int32_t randomTriIndex = rand() % navMesh.TriangleCount();
		glm::vec3 spawnPos = navMesh.GetTriangles()[randomTriIndex].centroid;

		// Add a small upward offset so banana doesn't spawn inside the ground
		spawnPos.y += 1.0f;

		// Spawn banana peel entity
		Entity entity = controller.createEntity();
		controller.AddComponent(entity, Transform{ spawnPos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.5f) });
		controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 1.0f, 1.0f });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.AddComponent(entity, Banana{});

		std::cout << "  Banana peel " << (i + 1) << "/" << count
			<< " at (" << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ")" << std::endl;
	}
}

void LevelLoaderSystem::SpawnRandomPowerups(int count, int powerupType, const NavMesh& navMesh)
{
	if (navMesh.IsEmpty())
	{
		std::cout << "[LevelLoader] Cannot spawn powerups: NavMesh is empty" << std::endl;
		return;
	}

	// Determine model path and scale based on powerup type
	std::string modelPath;
	glm::vec3 scale;
	std::string typeName;

	if (powerupType == 1)
	{
		modelPath = "assets/models/lightning_capsule/scene.gltf";
		scale = glm::vec3(0.25f);
		typeName = "Speed Boost";
	}
	else if (powerupType == 2)
	{
		modelPath = "assets/models/banana/scene.gltf";
		scale = glm::vec3(1.0f);
		typeName = "Banana Pickup";
	}
	else
	{
		std::cout << "[LevelLoader] Invalid powerup type: " << powerupType << std::endl;
		return;
	}

	auto loaded = LoadModel(modelPath);

	std::cout << "[LevelLoader] Spawning " << count << " random " << typeName << " powerups..." << std::endl;

	for (int i = 0; i < count; ++i)
	{
		// Get a random triangle from the navmesh
		int32_t randomTriIndex = rand() % navMesh.TriangleCount();
		glm::vec3 spawnPos = navMesh.GetTriangles()[randomTriIndex].centroid;

		// Add a small upward offset so powerup doesn't spawn inside the ground
		spawnPos.y += 2.0f;

		// Spawn powerup entity
		Entity entity = controller.createEntity();
		controller.AddComponent(entity, Transform{ spawnPos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), scale });
		controller.AddComponent(entity, Trigger{ nullptr, 1.0f, 2.0f, 1.0f });
		controller.AddComponent(entity, Render{ loaded.first, loaded.second, true });
		controller.AddComponent(entity, PhysicsBody{});
		controller.AddComponent(entity, Powerup{
			powerupType,
			false,  // not active
			5.0f,   // duration
			0.0f    // timer
		});

		std::cout << "  " << typeName << " " << (i + 1) << "/" << count
			<< " at (" << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ")" << std::endl;
	}
}

void LevelLoaderSystem::SpawnRandomMixedPowerups(int speedBoostCount, int bananaCount, const NavMesh& navMesh)
{
	std::cout << "[LevelLoader] Spawning mixed powerups: "
		<< speedBoostCount << " Speed Boosts, "
		<< bananaCount << " Banana Pickups" << std::endl;

	SpawnRandomPowerups(speedBoostCount, 1, navMesh);  // Speed boosts
	SpawnRandomPowerups(bananaCount, 2, navMesh);      // Banana pickups
}