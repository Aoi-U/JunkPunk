#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "CameraControlSystem.h""

#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../ECSController.h"

extern ECSController controller;

void CameraControlSystem::Init(std::shared_ptr<Gamepad> gamepad)
{
	this->gamepad = gamepad;
	controller.AddEventListener(Events::Window::RESIZED, [this](Event& e) { this->WindowSizeListener(e); });
}


void CameraControlSystem::Update(float deltaTime)
{
	Entity player = controller.GetEntityByTag("VehicleCommands");
	auto& playerTransform = controller.GetComponent<Transform>(player);
	auto& playerComp = controller.GetComponent<VehicleCommands>(player);

	for (auto const& entity : entities)
	{
		auto& camera = controller.GetComponent<ThirdPersonCamera>(entity);
		auto& transform = controller.GetComponent<Transform>(entity);

		if (gamepad->Connected())
		{
			if (!gamepad->RStick_InDeadzone())
			{
				// orbit camera around player
				camera.yaw +=  gamepad->RightStick_X() * camera.horizontalLookSpeed * deltaTime;
				camera.pitch += gamepad->RightStick_Y() * camera.verticalLookSpeed * deltaTime;
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