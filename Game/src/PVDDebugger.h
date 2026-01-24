#pragma once

#include <iostream>
#include <vector>

#include "PxPhysicsAPI.h"


class PVDDebugger
{
public:
	PVDDebugger() = default;
	~PVDDebugger() = default;

	int Init();

	void Update();

	physx::PxVec3 GetBoxPosition(int i);

private:
	//PhysX management class instances.
	physx::PxDefaultAllocator gAllocator;
	physx::PxDefaultErrorCallback gErrorCallback;
	physx::PxFoundation* gFoundation = NULL;
	physx::PxPhysics* gPhysics = NULL;
	physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
	physx::PxScene* gScene = NULL;
	physx::PxMaterial* gMaterial = NULL;
	physx::PxPvd* gPvd = NULL;


	std::vector<physx::PxRigidDynamic*> rigidDynamicList;
};