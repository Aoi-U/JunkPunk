#pragma once

#include <iostream>
#include <unordered_map>
#include <PxPhysicsAPI.h>

#include "../Core/Vehicle/MainVehicle.h"
#include "../Core/Mesh.h"
#include "../Core/Types.h"
#include "System.h"

class Event;

using namespace physx;
class PhysicsSystem : public System
{
public:
	PhysicsSystem();

	void Init();

	void Update(float deltaTime);

	void Plane();

	void Box(float halfLen, PxU32 size, PxVec3 position); // test


	void Simulate(float deltaTime);

	void Cleanup();

	MainVehicle& getVehicle() { return gVehicle; }

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
	MainVehicle gVehicle;

	void CreateMap();

	PxTriangleMesh* CreateTriangleMesh(const Mesh& mesh); // generate static triangle meshes from models

	void JumpEventListener(Event& e);

};
