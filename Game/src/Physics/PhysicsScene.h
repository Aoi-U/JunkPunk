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

	void Plane();

	void InitPhysicsComponentFromEntity(const Entity* entity);

	void Box(float halfLen, PxU32 size, PxVec3 position); // test

	void InitPhysics();

	void Simulate(float deltaTime);

	void Cleanup();

	Vehicle& getVehicle() { return gVehicle; }

	PxVec3 GetDynamicActorPos(std::string name);

	const PxRenderBuffer& GetRenderBuffer() { return gPhysicsScene->getRenderBuffer(); }

private:
	PxDefaultAllocator gAllocator;
	PxDefaultErrorCallback gErrorCallback;
	PxFoundation* gFoundation = NULL;
	PxPhysics* gPhysics = NULL;
	PxDefaultCpuDispatcher* gDispatcher = NULL;
	PxScene* gPhysicsScene = NULL;
	PxMaterial* gGroundMaterial = NULL;
	PxPvd* gPvd = NULL;

	PxRigidStatic* gGroundPlane = NULL;
	Vehicle gVehicle;

	PxTriangleMesh* CreateTriangleMesh(const Mesh& mesh); // generate static triangle meshes from models

	void CreateStaticPhysicsComponent(const Entity* entity);

	std::unordered_map<std::string, PxRigidDynamic*> dynamicActors;

};
