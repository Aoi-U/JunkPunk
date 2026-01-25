#pragma once

#include <PxPhysicsAPI.h>

using namespace physx;

class PhysicsCallbacks :public PxSimulationEventCallback
{
public:
	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override;
	void onWake(PxActor** actors, PxU32 count) override;
	void onSleep(PxActor** actors, PxU32 count) override;
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override;
	void onTrigger(PxTriggerPair* pairs, PxU32 count) override;
};