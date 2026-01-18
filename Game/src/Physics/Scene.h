#pragma once

#include <iostream>
#include <PxPhysicsAPI.h>

#include "Vehicle/Vehicle.h"
#include "../Entity.h"

using namespace physx;
class Scene
{
public:
	Scene();

	void InitPVD();

	void InitScene();

	void PrepPVD();

	void Plane();

	void Map(std::vector<Entity>& entities);

	void Box(float halfLen, PxU32 size, PxVec3 position); // test

	void InitPhysics(std::vector<Entity>& entities);

	void Simulate(float deltaTime);

	void Cleanup();

	Vehicle& getVehicle() { return gVehicle; }

	const PxRenderBuffer& GetRenderBuffer() { return gScene->getRenderBuffer(); }

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


	PxTriangleMesh* CreateTriangleMesh(Mesh& mesh); // generate static triangle meshes from models
};
