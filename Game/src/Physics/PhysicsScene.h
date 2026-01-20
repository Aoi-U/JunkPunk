#pragma once

#include <iostream>
#include <PxPhysicsAPI.h>

#include "Vehicle/Vehicle.h"
#include "../Entity.h"

using namespace physx;
class PhysicsScene
{
public:
	PhysicsScene();

	void InitPVD();

	void InitPhysicsScene();

	void PrepPVD();

	void Plane();

	void Map(std::vector<std::shared_ptr<Entity>> entities);

	void Box(float halfLen, PxU32 size, PxVec3 position); // test

	void InitPhysics(std::vector<std::shared_ptr<Entity>> entities);

	void Simulate(float deltaTime);

	void Cleanup();

	Vehicle& getVehicle() { return gVehicle; }

	const PxRenderBuffer& GetRenderBuffer() { return gPhysicsScene->getRenderBuffer(); }

private:
	PxDefaultAllocator gAllocator;
	PxDefaultErrorCallback gErrorCallback;
	PxFoundation* gFoundation = NULL;
	PxPhysics* gPhysics = NULL;
	PxDefaultCpuDispatcher* gDispatcher = NULL;
	PxScene* gPhysicsScene = NULL;
	PxMaterial* gMaterial = NULL;
	PxPvd* gPvd = NULL;

	PxRigidStatic* gGroundPlane = NULL;
	Vehicle gVehicle;


	PxTriangleMesh* CreateTriangleMesh(Mesh& mesh); // generate static triangle meshes from models
};
