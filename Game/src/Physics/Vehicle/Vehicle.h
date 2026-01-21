#pragma once

#include <iostream>
#include <PxPhysicsAPI.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include "directdrivetrain/DirectDrivetrain.h"
#include "enginedrivetrain/EngineDrivetrain.h"
#include "serialization/BaseSerialization.h"
//#include "serialization/DirectDrivetrainSerialization.h"
#include "serialization/EngineDrivetrainSerialization.h"
#include "SnippetVehicleHelpers.h"

//#include "VehicleParamHelper.h"

#include "glad/glad.h"


using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

struct Command
{
	PxF32 brake = 0.0;
	PxF32 throttle = 0.0;
	PxF32 steer = 0.0;
	PxU32 gear = 1;
	PxU32 duration;
};

class Vehicle
{
public:
	Vehicle();

	bool setup(PxScene* scene, PxPhysics* physics, PxMaterial* material);


	void step(float deltaTime);

	void cleanup();

	void setCommand(Command command);

	// ----------- game related functions -------------

	const PxTransform getTransform() const; // returns the vehicles transform matrix

	void resetTransform(); // use when vehicle is stuck (maybe dont need because of checkpoints)
	
	void setCheckpoint(const glm::vec3& position, const glm::vec3& rotation); // sets a new checkpoint position

	void respawnAtCheckpoint(); // respawns the vehicle at the last checkpoint position

	const PxVec3 getVelocity() const;

	void jump(); // makes the vehicle jump


private:
	//DirectDriveVehicle gVehicle;
	EngineDriveVehicle gVehicle;

	PxVehiclePhysXSimulationContext gVehicleSimulationContext;

	// The mapping between PxMaterial and friction
	PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
	PxU32 gNbPhysXmaterialFrictions = 0;
	PxReal gPhysXDefaultMaterialFriction = 1.0f;

	const char gVehicleName[9] = "Vehicle";

	Command gCommand;

	void initMaterialFrictionTable(PxMaterial* gMaterial);

	bool initVehicles(PxScene* gScene, PxPhysics* gPhysics, PxMaterial* gMaterial);

	// game related methods and variables 

	glm::mat4 gVehicleTransform = glm::mat4(1.0f);

	glm::vec3 checkpointPosition = glm::vec3(0.0f);
	glm::vec3 checkpointRotation = glm::vec3(0.0f);

	PxVec3 jumpForce = PxVec3(0.0f, 1000.0f, 0.0f);

	void setTransform(const glm::vec3& position, const glm::vec3& rotation); // set the position and rotation of the vehicle (use for checkpoints/respawning)
};