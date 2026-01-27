#include "VehicleControlSystem.h"
#include <iostream>
#include "../Components/Player.h"
#include "../ECSController.h"
#include "../Components/Physics.h"

extern ECSController controller;

void VehicleControlSystem::Init(std::shared_ptr<Gamepad> gamepad)
{
	this->gamepad = gamepad;
	controller.AddEventListener(Events::Window::INPUT, [this](Event& e) { this->KeyboardInputListener(e); });
}

void VehicleControlSystem::Update()
{
	if (gamepad)
	{
		gamepad->RefreshState();
		gamepad->Update();
	}

	for (auto& entity : entities)
	{
		auto& playerCommands = controller.GetComponent<VehicleCommands>(entity);
		
		if (gamepad->Connected())
		{
			playerCommands.throttle = gamepad->LeftTrigger();
			playerCommands.brake = gamepad->RightTrigger();
			playerCommands.steer = gamepad->LStick_InDeadzone() ? 0.0f : -gamepad->LeftStick_X();

			// send a jump event only when the jump button is pressed and the player is grounded
			if (playerCommands.isGrounded && gamepad->GetButtonDown(Buttons::JUMP))
			{
				std::cout << "Jump button pressed" << std::endl;

				// send jump event to physics and audio (can add more)
				Event event(Events::Player::PLAYER_JUMPED);
				event.SetParam<Entity>(Events::Player::Player_Jumped::ENTITY, entity);
				controller.SendEvent(event);
			}

			// send reset vehicle event when reset button is pressed
			if (gamepad->GetButtonDown(Buttons::RESET))
			{
				// send event to physics system
				Event event(Events::Player::RESET_VEHICLE);
				event.SetParam<Entity>(Events::Player::Reset_Vehicle::ENTITY, entity);
				controller.SendEvent(event);
			}
		}
	}
}

void VehicleControlSystem::KeyboardInputListener(Event& e)
{
	int keyRecieve = e.GetParam<int>(Events::Window::Input::KEY);
	int action = e.GetParam<bool>(Events::Window::Input::ACTION);
	char key = static_cast<char>(keyRecieve);

	for (auto& entity : entities)
	{
		VehicleCommands& playerCommands = controller.GetComponent<VehicleCommands>(entity);
		
		if (key == Keys::KEY_FORWARD)
		{
			playerCommands.throttle = action ? 1.0f : 0.0f;
		}
		if (key == Keys::KEY_BACKWARD)
		{
			playerCommands.brake = action ? 1.0f : 0.0f;
		}
		if (key == Keys::KEY_LEFT)
		{
			playerCommands.steer = action ? 1.0f : 0.0f;
		}
		if (key == Keys::KEY_RIGHT)
		{
			playerCommands.steer = action ? -1.0f : 0.0f;
		}
		if (key == Keys::KEY_JUMP)
		{
			if (playerCommands.isGrounded && action)
			{
				std::cout << "Jump key pressed" << std::endl;
				// send jump event to physics and audio (can add more)
				Event event(Events::Player::PLAYER_JUMPED);
				event.SetParam<Entity>(Events::Player::Player_Jumped::ENTITY, entity);
				controller.SendEvent(event);
			}
		}
		if (key == Keys::KEY_RESET)
		{
			if (action)
			{
				// send event to physics system
				Event event(Events::Player::RESET_VEHICLE);
				event.SetParam<Entity>(Events::Player::Reset_Vehicle::ENTITY, entity);
				controller.SendEvent(event);
			}
		}
	}
}
