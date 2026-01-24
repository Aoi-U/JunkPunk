#include "VehicleControlSystem.h"

#include "../Components/Player.h"
#include "../ECSController.h"

extern ECSController controller;

void VehicleControlSystem::Init(std::shared_ptr<Gamepad> gamepad)
{
	this->gamepad = gamepad;
}

void VehicleControlSystem::Update()
{
	for (auto& entity : entities)
	{
		auto& playerCommands = controller.GetComponent<Player>(entity);
		

		if (gamepad->Connected())
		{
			playerCommands.throttle = gamepad->LeftTrigger();
			playerCommands.brake = gamepad->RightTrigger();
			playerCommands.steer = gamepad->LStick_InDeadzone() ? 0.0f : -gamepad->LeftStick_X();
			
			playerCommands.jump = gamepad->GetButtonDown(Buttons::JUMP);
		}
	}
}