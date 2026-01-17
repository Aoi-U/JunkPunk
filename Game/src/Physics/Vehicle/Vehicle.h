#pragma once

#include <iostream>
#include <PxPhysicsAPI.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "directdrivetrain/DirectDrivetrain.h"
//#include "serialization/BaseSerialization.h"
//#include "serialization/DirectDrivetrainSerialization.h"
#include "SnippetVehicleHelpers.h"

#include "VehicleParamHelper.h"

#include "glad/glad.h"


using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;

struct Command
{
	PxF32 brake = 0.0;
	PxF32 throttle = 0.0;
	PxF32 steer = 0.0;
};

class Vehicle
{
public:
	Vehicle();

	void initMaterialFrictionTable(PxMaterial* gMaterial);

	bool initVehicles(PxScene* gScene, PxPhysics* gPhysics, PxMaterial* gMaterial);

	void step(float deltaTime);

	void cleanup();

	void setCommand(Command command);

	glm::mat4 getTransform() const;

	// get the wireframe mesh for rendering

private:
	DirectDriveVehicle gVehicle;

	PxVehiclePhysXSimulationContext gVehicleSimulationContext;

	// The mapping between PxMaterial and friction
	PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
	PxU32 gNbPhysXmaterialFrictions = 0;
	PxReal gPhysXDefaultMaterialFriction = 1.0f;

	const char gVehicleName[9] = "Vehicle";

	Command gCommand;
};