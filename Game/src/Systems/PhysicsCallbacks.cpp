#include "PhysicsCallbacks.h"

void PhysicsCallbacks::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
{
}

void PhysicsCallbacks::onWake(PxActor** actors, PxU32 count)
{
}

void PhysicsCallbacks::onSleep(PxActor** actors, PxU32 count)
{
}

void PhysicsCallbacks::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
}

void PhysicsCallbacks::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
}
