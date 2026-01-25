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
		
			if (playerCommands.isGrounded && gamepad->GetButtonDown(Buttons::JUMP))
			{
				std::cout << "Jump button pressed" << std::endl;

				Event event(Events::Player::PLAYER_JUMPED);
				event.SetParam<Entity>(Events::Player::Player_Jumped::ENTITY, entity);
				controller.SendEvent(event);
			}
		}
	}
}