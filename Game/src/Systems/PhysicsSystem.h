#pragma once

#include <iostream>
#include <unordered_map>
#include <PxPhysicsAPI.h>

#include "../Core/Vehicle/MainVehicle.h"
#include "../Core/Mesh.h"
#include "../Core/Types.h"
#include "../Core/PhysicsCallbacks.h"
#include "../Components/Physics.h"
#include "System.h"

class Event;

using namespace physx;
class PhysicsSystem : public System
{
public:
	PhysicsSystem();

	void Init();

	void Update(float deltaTime);

	void Cleanup();

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

	PhysicsCallbacks gPhysicsCallbacks;

	std::unordered_map<std::string, PxMaterial*> materialMap; // holds different physics materials

	void Simulate(float deltaTime); // run a physics simulation step

	void CreateMap(); // initialize physics map

	PxTriangleMesh* CreateTriangleMesh(const Mesh& mesh); // generate static triangle meshes from models



	// listeners


	void CreateActorListener(Event& e); // listens for a create actor event

	void JumpEventListener(Event& e); // listens for jump events

	void ResetVehicleEventListener(Event& e); // listens for vehicle reset events

	// callbacks

	void ReleaseActorCallback(Entity entity, RigidBody& rb); // automatically called when an entity is destroyed or rigidbody is removed
};
