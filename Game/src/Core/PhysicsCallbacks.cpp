#include "PhysicsCallbacks.h"
#include "../ECSController.h"
#include "../Core/Types.h"
#include <iostream>

extern ECSController controller;

void PhysicsCallbacks::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	// https://nvidia-omniverse.github.io/PhysX/physx/5.6.1/_api_build/classPxSimulationEventCallback.html
	// https://nvidia-omniverse.github.io/PhysX/physx/5.6.1/_api_build/structPxShapeFlag.html#structpxshapeflag_1a6edb481aaa3a998c5d6dd3fc4ad87f1aaef2b90024dc86be72b68bbaf94a5821d
	// implementation should send an event to the ECSController with necessary parameters so 
	// other systems can execute their logic
	// since we will probably use triggers for other stuff like picking up powerups, getting checkpoints, etc.
	// i put guides for using events in Core/Types.h
	
	for (PxU32 i = 0; i < count; i++) {
		const PxTriggerPair& pair = pairs[i];
		/*if (!(pair.status & PxPairFlag::eNOTIFY_TOUCH_FOUND))
			continue;*/

		if (!pair.triggerActor->userData || !pair.otherActor->userData)
			continue;

		Entity triggerEntity = static_cast<Entity>(reinterpret_cast<uintptr_t>(pair.triggerActor->userData));
		Entity otherEntity = static_cast<Entity>(reinterpret_cast<uintptr_t>(pair.otherActor->userData));
		if (pair.status & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
			Event e(Events::Physics::TRIGGER_ENTER);
			e.SetParam<Entity>(Events::Physics::Trigger_Enter::ENTITY_ONE, triggerEntity);
			e.SetParam<Entity>(Events::Physics::Trigger_Enter::ENTITY_TWO, otherEntity);
			controller.SendEvent(e);
			std::cout << "trigger event was called" << std::endl;
		}

		if (pair.status & PxPairFlag::eNOTIFY_TOUCH_LOST) {
			Event e(Events::Physics::TRIGGER_EXIT);
			e.SetParam<Entity>(Events::Physics::Trigger_Enter::ENTITY_ONE, triggerEntity);
			e.SetParam<Entity>(Events::Physics::Trigger_Enter::ENTITY_TWO, otherEntity);
			controller.SendEvent(e);
			std::cout << "exited trigger" << std::endl;
		}
	}
}
