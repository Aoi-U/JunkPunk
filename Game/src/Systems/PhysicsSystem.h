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

	bool usedBoost = false;
	Entity spinningEntity = MAX_ENTITIES;
	float spinTimer = 0.0f;

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
	//std::unique_ptr<MainVehicle> gVehicle = NULL;

	std::unordered_map<Entity, std::unique_ptr<MainVehicle>> vehicles; // map of vehicle entities to their MainVehicle instances 

	PhysicsCallbacks gPhysicsCallbacks;

	std::unordered_map<std::string, PxMaterial*> materialMap; // holds different physics materials

	std::vector<PxActor*> actorsToDelete; // list of actors to delete 

	std::unordered_map<Entity, bool> playerOutOfBounds; // tracks whether each player is out of bounds for respawn purposes

	//bool usedBoost = false;
	//bool spinning = false;
	//float spinTimer = 0.0f;

	void DeleteActorsQueue();

	void Simulate(float deltaTime); // run a physics simulation step

	void CreateMap(); // initialize physics map

	PxTriangleMesh* CreateTriangleMesh(const Mesh& mesh); // generate static triangle meshes from models

	// listeners


	void CreateActorListener(Event& e); // listens for a create actor event

	void JumpEventListener(Event& e); // listens for jump events

	void ResetVehicleEventListener(Event& e); // listens for vehicle reset events
	
	void CheckpointReachedListener(Event& e); // listens for checkpoint events

	void SpinOutListener(Event& e);

	void BlastEventListener(Event& e);

	void PlayerOutOfBoundsListener(Event& e);

	// callbacks

	void ReleaseActorCallback(Entity entity, RigidBody& rb); // automatically called when an entity is destroyed or rigidbody is removed

	void ReleaseTriggerCallback(Entity entity, Trigger& trig); // automatically called when an entity is destroyed or trigger is removed
};
