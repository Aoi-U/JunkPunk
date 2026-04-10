#include "VehicleControlSystem.h"
#include <iostream>
#include "../Components/Player.h"
#include "../ECSController.h"
#include "../Components/Physics.h"

extern ECSController controller;

void VehicleControlSystem::Init(std::vector<std::shared_ptr<Gamepad>> gamepads)
{
	//this->gamepad = gamepad;
	for (int i = 0; i < (int)gamepads.size(); i++)
	{
		this->gamepads[i + 1] = gamepads[i]; 
	}
	controller.AddEventListener(Events::Window::INPUT, [this](Event& e) { this->KeyboardInputListener(e); });
}

void VehicleControlSystem::Update()
{
	for (auto& entity : entities)
	{
		auto& playerCommands = controller.GetComponent<VehicleCommands>(entity);
		auto& playerController = controller.GetComponent<PlayerController>(entity);

		auto it = gamepads.find(playerController.playerNum);
		if (it == gamepads.end()) continue; 

		auto& gamepad = it->second;

		if (gamepad->Connected())
		{
			playerCommands.steer = gamepad->LStick_InDeadzone() ? 0.0f : -gamepad->LeftStick_X();
			if (gamepad->RightTrigger() > 0.1f)
			{
				playerCommands.throttle = gamepad->RightTrigger();
				playerCommands.brake = 0.0f;
			}
			else if (gamepad->LeftTrigger() > 0.1f)
			{
				playerCommands.brake = gamepad->LeftTrigger();
				playerCommands.throttle = 0.0f;
			}
			else
			{
				playerCommands.throttle = 0.0f;
				playerCommands.brake = 0.0f;
			}

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
		auto& playerController = controller.GetComponent<PlayerController>(entity);

		VehicleCommands& playerCommands = controller.GetComponent<VehicleCommands>(entity);
		if (playerController.playerNum == 1) {
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
					//std::cout << "Jump key pressed" << std::endl;
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
		if (playerController.playerNum == 2) {
			if (key == Keys::KEY_2_FORWARD)
			{
				playerCommands.throttle = action ? 1.0f : 0.0f;
			}
			if (key == Keys::KEY_2_BACKWARD)
			{
				playerCommands.brake = action ? 1.0f : 0.0f;
			}
			if (key == Keys::KEY_2_LEFT)
			{
				playerCommands.steer = action ? 1.0f : 0.0f;
			}
			if (key == Keys::KEY_2_RIGHT)
			{
				playerCommands.steer = action ? -1.0f : 0.0f;
			}
			if (key == Keys::KEY_2_JUMP)
			{
				if (playerCommands.isGrounded && action)
				{
					//std::cout << "Jump key pressed" << std::endl;
					// send jump event to physics and audio (can add more)
					Event event(Events::Player::PLAYER_JUMPED);
					event.SetParam<Entity>(Events::Player::Player_Jumped::ENTITY, entity);
					controller.SendEvent(event);
				}
			}
			if (key == Keys::KEY_2_RESET)
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
		if (playerController.playerNum == 3) {
			if (key == Keys::KEY_3_FORWARD)
			{
				playerCommands.throttle = action ? 1.0f : 0.0f;
			}
			if (key == Keys::KEY_3_BACKWARD)
			{
				playerCommands.brake = action ? 1.0f : 0.0f;
			}
			if (key == Keys::KEY_3_LEFT)
			{
				playerCommands.steer = action ? 1.0f : 0.0f;
			}
			if (key == Keys::KEY_3_RIGHT)
			{
				playerCommands.steer = action ? -1.0f : 0.0f;
			}
			if (key == Keys::KEY_3_JUMP)
			{
				if (playerCommands.isGrounded && action)
				{
					//std::cout << "Jump key pressed" << std::endl;
					// send jump event to physics and audio (can add more)
					Event event(Events::Player::PLAYER_JUMPED);
					event.SetParam<Entity>(Events::Player::Player_Jumped::ENTITY, entity);
					controller.SendEvent(event);
				}
			}
			if (key == Keys::KEY_3_RESET)
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
		if (playerController.playerNum == 4) {
			if (keyRecieve == Keys::KEY_4_FORWARD)
			{
				playerCommands.throttle = action ? 1.0f : 0.0f;
			}
			if (keyRecieve == Keys::KEY_4_BACKWARD)
			{
				playerCommands.brake = action ? 1.0f : 0.0f;
			}
			if (keyRecieve == Keys::KEY_4_LEFT)
			{
				playerCommands.steer = action ? 1.0f : 0.0f;
			}
			if (keyRecieve == Keys::KEY_4_RIGHT)
			{
				playerCommands.steer = action ? -1.0f : 0.0f;
			}
			if (keyRecieve == Keys::KEY_4_JUMP)
			{
				if (playerCommands.isGrounded && action)
				{
					//std::cout << "Jump key pressed" << std::endl;
					// send jump event to physics and audio (can add more)
					Event event(Events::Player::PLAYER_JUMPED);
					event.SetParam<Entity>(Events::Player::Player_Jumped::ENTITY, entity);
					controller.SendEvent(event);
				}
			}
			if (keyRecieve == Keys::KEY_4_RESET)
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
}
