#pragma once

#include "PxPhysicsAPI.h"
#include "vehicle2/PxVehicleAPI.h"
#include <memory>
#include <iostream>

using namespace physx;
using namespace physx::vehicle2;

class Vehicle
{
public:
	Vehicle();

	void Simulate(float deltaTime);

private:
	physx::PxDefaultAllocator gAllocator;
	physx::PxDefaultErrorCallback gErrorCallback;
	physx::PxFoundation* gFoundation = NULL;
	physx::PxPhysics* gPhysics = NULL;
	physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
	physx::PxScene* gScene = NULL;
	physx::PxMaterial* gMaterial = NULL;
	physx::PxPvd* gPvd = NULL;


};
