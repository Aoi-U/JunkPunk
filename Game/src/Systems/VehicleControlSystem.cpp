#include "VehicleControlSystem.h"
#include <iostream>
#include "../Components/Player.h"
#include "../ECSController.h"

extern ECSController controller;

void VehicleControlSystem::Init(std::shared_ptr<Gamepad> gamepad)
{
	this->gamepad = gamepad;
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