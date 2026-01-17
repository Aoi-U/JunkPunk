#pragma once

#include <iostream>
#include <PxPhysicsAPI.h>

#include "Vehicle/Vehicle.h"

using namespace physx;
class Scene
{
public:
	Scene();

	void InitPVD();

	void InitScene();

	void PrepPVD();

	void Plane();

	void Box(float halfLen, PxU32 size, PxVec3 position); // test

	void InitPhysics();

	void Simulate(float deltaTime);

	void Cleanup();

	Vehicle& getVehicle() { return gVehicle; }

private:
	PxDefaultAllocator gAllocator;
	PxDefaultErrorCallback gErrorCallback;
	PxFoundation* gFoundation = NULL;
	PxPhysics* gPhysics = NULL;
	PxDefaultCpuDispatcher* gDispatcher = NULL;
	PxScene* gScene = NULL;
	PxMaterial* gMaterial = NULL;
	PxPvd* gPvd = NULL;

	PxRigidStatic* gGroundPlane = NULL;
	Vehicle gVehicle;
};
