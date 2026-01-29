#include "PhysicsCallbacks.h"

void PhysicsCallbacks::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
{ // probably dont need
}

void PhysicsCallbacks::onWake(PxActor** actors, PxU32 count)
{ // probably dont need
}

void PhysicsCallbacks::onSleep(PxActor** actors, PxU32 count)
{ // probably dont need
}

void PhysicsCallbacks::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{ // probably dont need
}

void PhysicsCallbacks::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	// implementation should send an event to the ECSController with necessary parameters so 
	// other systems can execute their logic
	// since we will probably use triggers for other stuff like picking up powerups, getting checkpoints, etc.
	// i put guides for using events in Core/Types.h
}
