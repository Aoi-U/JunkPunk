#pragma once

#include <iostream>
#include <PxPhysicsAPI.h>
#include <vector>
#include <iostream>

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

#include "../Types.h"


using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

struct Command
{
	PxF32 brake = 0.0;
	PxF32 throttle = 0.0;
	PxF32 steer = 0.0;
	PxU32 gear = 1;
	PxU32 duration = 0.0f;
};

class MainVehicle 
{
public:
	MainVehicle();

	bool setup(PxScene* scene, PxPhysics* physics, PxMaterial* material);


	void step(float deltaTime);

	void setEntityUserData(Entity entity);

	void cleanup();

	void setCommand(Command command);

	// ----------- game related functions -------------

	const PxTransform getTransform() const; // returns the vehicles transform matrix

	void resetTransform(); // use when vehicle is stuck (maybe dont need because of checkpoints)
	
	void setCheckpoint(const glm::vec3& position, const glm::quat& rotation); // sets a new checkpoint position

	void respawnAtCheckpoint(); // respawns the vehicle at the last checkpoint position

	const PxVec3 getLinearVelocity() const;

	const PxVec3 getAngularVelocity() const;

	bool IsGrounded(PxScene* scene) const;

	void jump(); // makes the vehicle jump

	void ApplyBoost(float multiplier);
	void ClearBoost();

	void SpinOut();

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

	glm::vec3 checkpointPosition = glm::vec3(0.0f, -30.0f, 0.0f);
	glm::quat checkpointRotation = glm::vec3(0.0f);

	PxVec3 jumpForce = PxVec3(0.0f, 20000.0f, 0.0f);

	void setTransform(const glm::vec3& position, const glm::quat& rotation); // set the position and rotation of the vehicle (use for checkpoints/respawning)
	
	float basePeakTorque = -1.0f;
};