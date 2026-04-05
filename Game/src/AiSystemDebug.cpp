#include "AiSystemDebug.h"
#include "./Systems/AiSystem.h"
#include "./ECSController.h"
#include "./Components/Transform.h"
#include <iostream>
#include "./Components/AiDriver.h"
#include "./Components/Player.h"
#include "./Components/Physics.h"
#include "./Components/Obstacle.h"  // for CheckPoint
// ... other includes

extern ECSController controller;

void AiSystemDebug::SpawnDebugWaypoints(AiSystem& aiSystem, Entity aiEntity)
{
	aiSystem.ComputeNavPath(aiEntity);

	auto& ai = controller.GetComponent<AiDriver>(aiEntity);

	std::cout << "[AiSystem] Spawning " << ai.navWaypoints.size() << " debug waypoint markers" << std::endl;

	for (size_t i = 0; i < ai.navWaypoints.size(); ++i)
	{
		Entity marker = controller.createEntity();
		controller.AddComponent(marker, PhysicsBody{});
		controller.AddComponent(marker, Transform{
			ai.navWaypoints[i],
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec3(0.25f)
			});
		controller.AddComponent(marker, CheckPoint{ glm::quat(1.0f, 0.0f, 0.0f, 0.0f) });
		controller.AddComponent(marker, Trigger{ nullptr, 1.0f, 4.0f, 1.0f });

		std::cout << "  marker[" << i << "]: (" << ai.navWaypoints[i].x << ", "
			<< ai.navWaypoints[i].y << ", " << ai.navWaypoints[i].z << ")" << std::endl;
	}
}

void AiSystemDebug::SpawnDebugZoneTriggers(AiSystem& aiSystem)
{
	std::cout << "[AiSystem] Spawning debug zone trigger boxes" << std::endl;

	// BOXING GLOVE ZONE
	{
		glm::vec3 minBounds(-76.0f, -178.0f, -74.0f);
		glm::vec3 maxBounds(48.0f, -173.0f, -41.0f);

		glm::vec3 center = (minBounds + maxBounds) * 0.5f;
		glm::vec3 dimensions = maxBounds - minBounds;

		Entity gloveZone = controller.createEntity();
		controller.AddComponent(gloveZone, PhysicsBody{});
		controller.AddComponent(gloveZone, Transform{
			center,
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec3(1.0f)
			});
		controller.AddComponent(gloveZone, Trigger{
			nullptr,
			dimensions.x * 0.5f,
			dimensions.y * 0.5f,
			dimensions.z * 0.5f
			});

		std::cout << "  [BOXING GLOVE ZONE] Center: (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
		std::cout << "                      Size: (" << dimensions.x << ", " << dimensions.y << ", " << dimensions.z << ")" << std::endl;
	}

	// GAP ZONE
	{
		glm::vec3 minBounds(-65.0f, -35.0f, 138.0f);
		glm::vec3 maxBounds(155.0f, -25.0f, 219.0f);

		glm::vec3 center = (minBounds + maxBounds) * 0.5f;
		glm::vec3 dimensions = maxBounds - minBounds;

		Entity gapZone = controller.createEntity();
		controller.AddComponent(gapZone, PhysicsBody{});
		controller.AddComponent(gapZone, Transform{
			center,
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec3(1.0f)
			});
		controller.AddComponent(gapZone, Trigger{
			nullptr,
			dimensions.x * 0.5f,
			dimensions.y * 0.5f,
			dimensions.z * 0.5f
			});

		std::cout << "  [GAP ZONE] Center: (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
		std::cout << "             Size: (" << dimensions.x << ", " << dimensions.y << ", " << dimensions.z << ")" << std::endl;
	}

	// TUNNEL ZONE
	//{
	//	glm::vec3 minBounds(-145.0f, -35.0f, 235.0f);
	//	glm::vec3 maxBounds(160.0f, -20.0f, 405.0f);

	//	glm::vec3 center = (minBounds + maxBounds) * 0.5f;
	//	glm::vec3 dimensions = maxBounds - minBounds;

	//	Entity tunnelZone = controller.createEntity();
	//	controller.AddComponent(tunnelZone, PhysicsBody{});
	//	controller.AddComponent(tunnelZone, Transform{
	//		center,
	//		glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
	//		glm::vec3(1.0f)
	//		});
	//	controller.AddComponent(tunnelZone, Trigger{
	//		nullptr,
	//		dimensions.x * 0.5f,
	//		dimensions.y * 0.5f,
	//		dimensions.z * 0.5f
	//		});

	//	std::cout << "  [TUNNEL ZONE] Center: (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
	//	std::cout << "                Size: (" << dimensions.x << ", " << dimensions.y << ", " << dimensions.z << ")" << std::endl;
	//}
}