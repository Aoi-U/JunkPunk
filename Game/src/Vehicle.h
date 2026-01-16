#pragma once

#include <memory>
#include <iostream>

#include "PxPhysicsAPI.h"
#include "vehicle2/PxVehicleAPI.h"
#include "BaseVehicle.h"
#include "EngineDriveTrain.h"

using namespace physx;
using namespace physx::vehicle2;

class Vehicle
{
public:
	Vehicle();

	void InitPhysX();

	void InitGroundPlane();

	void InitMaterialFrictionTable();

	bool InitVehicles();

	bool InitPhysics();

	void stepPhysics();

	void Simulate(float deltaTime);

	void Cleanup();

private:
	#define PVD_HOST "127.0.0.1"	//Set this to the IP address of the system running the PhysX Visual Debugger that you want to connect to.

	PxDefaultAllocator gAllocator;
	PxDefaultErrorCallback gErrorCallback;
	PxFoundation* gFoundation = NULL;
	PxPhysics* gPhysics = NULL;
	PxDefaultCpuDispatcher* gDispatcher = NULL;
	PxScene* gScene = NULL;
	PxMaterial* gMaterial = NULL;
	PxPvd* gPvd = NULL;

	PxRigidStatic* gGroundPlane = NULL;

	//The mapping between PxMaterial and friction.
	PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
	PxU32 gNbPhysXMaterialFrictions = 0;
	PxReal gPhysXDefaultMaterialFriction = 1.0f;

	//Give the vehicle a name so it can be identified in PVD.
	const char gVehicleName[12] = "engineDrive";

	//Commands are issued to the vehicle in a pre-choreographed sequence.
	struct Command
	{
		PxF32 brake0;			//Tanks have two brake controllers:
		PxF32 brake1;			//  one brake controller for the left track and one for the right track.
		PxF32 thrust0;			//Tanks have two thrust controllers that divert engine torque to the left and right tracks:
		PxF32 thrust1;			//  one thrust controller for the left track and one for the right track.
		PxF32 throttle;			//Tanks are driven by an engine that requires a throttle to generate engine drive torque.
		PxU32 gear;				//Tanks are geared and may use automatic gearing.
		PxF32 duration;
	};	

	PxVehiclePhysXSimulationContext gVehicleSimulationContext;


	const PxVec3 gGravity{ 0.0f, -9.81f, 0.0f };

	snippetvehicle::EngineDriveVehicle gVehicle;

	void cleanupPhysX();
};
