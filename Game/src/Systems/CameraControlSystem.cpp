#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "CameraControlSystem.h""

#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../ECSController.h"
#include "../Components/Powerup.h"
#include "../Components/Physics.h"

extern ECSController controller;

void CameraControlSystem::Init(std::vector<std::shared_ptr<Gamepad>> gamepads)
{
	for (int i = 0; i < (int)gamepads.size(); i++)
	{
		this->gamepads[i + 1] = gamepads[i]; // playerNum is 1-based
	}
	controller.AddEventListener(Events::Window::RESIZED, [this](Event& e) { this->WindowSizeListener(e); });
	controller.AddEventListener(Events::Window::MOUSEMOVED, [this](Event& e) { this->MouseMovedListener(e); });
}


void CameraControlSystem::Update(float deltaTime)
{
	for (int i = 0; i < numPlayers; i++)
	{
		Entity player = controller.GetEntityByTag("Player" + std::to_string(i + 1));

		bool boosting = false;
		if (controller.HasComponent<Powerup>(player)) {
			auto& p = controller.GetComponent<Powerup>(player);
			if (p.type == 1 && p.active)
				boosting = true;
		}
	}

	bool boosting = false;

	for (auto const& entity : entities)
	{
		auto& camera = controller.GetComponent<ThirdPersonCamera>(entity);
		auto& transform = controller.GetComponent<Transform>(entity);
		auto& playerTransform = controller.GetComponent<Transform>(camera.playerEntity);
		auto& vehicleComp = controller.GetComponent<VehicleBody>(camera.playerEntity);
	 
		float speed = glm::length(vehicleComp.linearVelocity);
		float camerad = glm::mix(camera.baseRadius, camera.baseRadius * 1.5f, glm::smoothstep(0.0f, 20.0f, speed)); // zoom out the camera asedon /speed using smoothstep for a smoother transition
	 
		float boostMultiplier = boosting ? 1.35f : 1.0f;
		float targetRadius = camerad * boostMultiplier;
		camera.radius = glm::mix(camera.radius, targetRadius, 5.0f * deltaTime);
	 
		auto& gamepad = gamepads[1]; // assuming single player for now
	 
		if (gamepad->Connected())
		{
	 		if (!gamepad->RStick_InDeadzone())
	 		{
	 			// orbit camera around player
	 			camera.yaw +=  -gamepad->RightStick_X() * camera.horizontalLookSpeed * deltaTime;
	 			camera.pitch += -gamepad->RightStick_Y() * camera.verticalLookSpeed * deltaTime;
	 			camera.pitch = glm::clamp(camera.pitch, glm::radians(-89.0f), glm::radians(89.0f));
	 		}
		}
	 		
		glm::vec3 playerForward = glm::normalize(playerTransform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f));
	 	
		float targetYaw = glm::atan(playerForward.x, playerForward.z);
		float deltaYaw = targetYaw - camera.yaw;
		deltaYaw = glm::mod(deltaYaw + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
	 
		// lerp the camera for smooth rotation
		float lerpFactor = camera.lerpSpeed * deltaTime;
		camera.yaw = camera.yaw + glm::mix(0.0f, deltaYaw, lerpFactor);
	 		
		camera.yaw = glm::mod(camera.yaw + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
	 
		glm::vec3 targetPosition = glm::vec3(playerTransform.position);
		targetPosition.y += camera.heightOffset;
	 
		glm::vec3 localOffset = glm::vec3(0.0f, camera.radius * glm::sin(camera.pitch), -camera.radius * glm::cos(camera.pitch));
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), camera.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 offset = glm::vec3(rotation * glm::vec4(localOffset, 0.0f));
		transform.position = targetPosition + offset;
	 
		camera.viewMatrix = glm::lookAt(transform.position, targetPosition, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	//Entity player = controller.GetEntityByTag("VehicleCommands");
	//
	//bool boosting = false;
	//if (controller.HasComponent<Powerup>(player)) {
	//	auto& p = controller.GetComponent<Powerup>(player);
	//	if (p.type == 1 && p.active)
	//		boosting = true;
	//}
	//
	//for (auto const& entity : entities)
	//{
	//	auto& camera = controller.GetComponent<ThirdPersonCamera>(entity);
	//	auto& transform = controller.GetComponent<Transform>(entity);
	//	auto& playerTransform = controller.GetComponent<Transform>(camera.playerEntity);
	//	auto& vehicleComp = controller.GetComponent<VehicleBody>(camera.playerEntity);
	//
	//	float speed = glm::length(vehicleComp.linearVelocity);
	//	float camerad = glm::mix(camera.baseRadius, camera.baseRadius * 1.5f, glm::smoothstep(0.0f, 20.0f, speed)); // zoom out the camera based /on /speed using smoothstep for a smoother transition
	//
	//	float boostMultiplier = boosting ? 1.35f : 1.0f;
	//	float targetRadius = camerad * boostMultiplier;
	//	camera.radius = glm::mix(camera.radius, targetRadius, 5.0f * deltaTime);
	//
	//	auto& gamepad = gamepads[1]; // assuming single player for now
	//
	//	if (gamepad->Connected())
	//	{
	//		if (!gamepad->RStick_InDeadzone())
	//		{
	//			// orbit camera around player
	//			camera.yaw +=  -gamepad->RightStick_X() * camera.horizontalLookSpeed * deltaTime;
	//			camera.pitch += -gamepad->RightStick_Y() * camera.verticalLookSpeed * deltaTime;
	//			camera.pitch = glm::clamp(camera.pitch, glm::radians(-89.0f), glm::radians(89.0f));
	//		}
	//	}
	//		
	//	glm::vec3 playerForward = glm::normalize(playerTransform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f));
	//	
	//	float targetYaw = glm::atan(playerForward.x, playerForward.z);
	//	float deltaYaw = targetYaw - camera.yaw;
	//	deltaYaw = glm::mod(deltaYaw + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
	//
	//	// lerp the camera for smooth rotation
	//	float lerpFactor = camera.lerpSpeed * deltaTime;
	//	camera.yaw = camera.yaw + glm::mix(0.0f, deltaYaw, lerpFactor);
	//		
	//	camera.yaw = glm::mod(camera.yaw + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
	//
	//	glm::vec3 targetPosition = glm::vec3(playerTransform.position);
	//	targetPosition.y += camera.heightOffset;
	//
	//	glm::vec3 localOffset = glm::vec3(0.0f, camera.radius * glm::sin(camera.pitch), -camera.radius * glm::cos(camera.pitch));
	//	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), camera.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	//	glm::vec3 offset = glm::vec3(rotation * glm::vec4(localOffset, 0.0f));
	//	transform.position = targetPosition + offset;
	//
	//	camera.viewMatrix = glm::lookAt(transform.position, targetPosition, glm::vec3(0.0f, 1.0f, 0.0f));
	//}
}

void CameraControlSystem::WindowSizeListener(Event& e)
{
	unsigned int width = e.GetParam<unsigned int>(Events::Window::Resized::WIDTH);
	unsigned int height = e.GetParam<unsigned int>(Events::Window::Resized::HEIGHT);

	// set camera screen size
	for (auto const& entity : entities)
	{
		auto& camera = controller.GetComponent<ThirdPersonCamera>(entity);
		camera.screenWidth = width;
		camera.screenHeight = height;
		std::cout << "Camera resized to: " << width << "x" << height << std::endl;
	}
}

void CameraControlSystem::MouseMovedListener(Event& e)
{	
	double xpos = e.GetParam<double>(Events::Window::Mouse_Moved::XPOS);
	double ypos = e.GetParam<double>(Events::Window::Mouse_Moved::YPOS);

	double deltaX = xpos - lastPosX;
	double deltaY = ypos - lastPosY;

	lastPosX = xpos;
	lastPosY = ypos;

	if (abs(deltaX) > 100.f || abs(deltaY) > 100.f) {//make less jumpy
		return;
	}

	// rotate camera based on mouse movement
	for (auto& entity : entities)
	{
		auto& camera = controller.GetComponent<ThirdPersonCamera>(entity);
		camera.yaw += deltaX * 0.005f;
		camera.pitch += deltaY * 0.005f;
		camera.pitch = glm::clamp(camera.pitch, glm::radians(-89.0f), glm::radians(89.0f));

	}
}
