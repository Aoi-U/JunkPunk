#include "Vehicle.h"

Vehicle::Vehicle()
{
}

void Vehicle::initMaterialFrictionTable(PxMaterial* gMaterial)
{
	gPhysXMaterialFrictions[0].friction = 1.0f;
	gPhysXMaterialFrictions[0].material = gMaterial;
	gPhysXDefaultMaterialFriction = 1.0f;
	gNbPhysXmaterialFrictions = 1;
}

bool Vehicle::initVehicles(PxScene* gScene, PxPhysics* gPhysics, PxMaterial* gMaterial)
{
	// Initialize the vehicle
	VehicleParamHelper vph;
	vph.setBaseParams(gVehicle.mBaseParams);
	vph.setPhysXIntegrationParams(gVehicle.mBaseParams.axleDescription, gPhysXMaterialFrictions, gNbPhysXmaterialFrictions, gPhysXDefaultMaterialFriction, gVehicle.mPhysXParams);
	vph.setDirectDriveParams(gVehicle.mDirectDriveParams);
	
	if (!gVehicle.initialize(*gPhysics, PxCookingParams(PxTolerancesScale()), *gMaterial))
	{
		std::cout << "Failed to initialize vehicle!" << std::endl;
		return false;
	}

	gVehicle.mTransmissionCommandState.gear = PxVehicleDirectDriveTransmissionCommandState::eFORWARD;

	// Apply a start pose to the physx actor and add it to the physx scene
	PxTransform pose(PxVec3(0.0f, 2.0f, 0.0f), PxQuat(PxIdentity));
	gVehicle.setUpActor(*gScene, pose, gVehicleName);

	PxShape* chassisShape = gPhysics->createShape(PxBoxGeometry(vph.physxActorBoxShapeHalfExtents), *gMaterial);
	chassisShape->setLocalPose(vph.physxActorCMassLocalPose);
	gVehicle.mPhysXState.physxActor.rigidBody->attachShape(*chassisShape);
	chassisShape->release();

	gVehicleSimulationContext.setToDefault();
	gVehicleSimulationContext.frame.lngAxis = PxVehicleAxes::ePosZ;
	gVehicleSimulationContext.frame.latAxis = PxVehicleAxes::ePosX;
	gVehicleSimulationContext.frame.vrtAxis = PxVehicleAxes::ePosY;
	gVehicleSimulationContext.scale.scale = 1.0f;
	gVehicleSimulationContext.gravity = gScene->getGravity();
	gVehicleSimulationContext.physxScene = gScene;
	gVehicleSimulationContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;

	gVehicle.mPhysXState.physxActor.rigidBody->setActorFlag(PxActorFlag::eVISUALIZATION, true);
	
	return true;
}

void Vehicle::step(float deltaTime)
{
	//Apply the brake, throttle and steer to the command state of the direct drive vehicle.
	gVehicle.mCommandState.brakes[0] = gCommand.brake;
	gVehicle.mCommandState.nbBrakes = 1;
	gVehicle.mCommandState.throttle = gCommand.throttle;
	gVehicle.mCommandState.steer = gCommand.steer;

	//Forward integrate the vehicle by a single timestep.
	gVehicle.step(1 / 60.0f, gVehicleSimulationContext);
}

void Vehicle::cleanup()
{
	gVehicle.destroy();
}

void Vehicle::setCommand(Command commands)
{
	gCommand = commands;
}

glm::mat4 Vehicle::getTransform() const
{
	PxTransform t = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();

	glm::quat rotation(t.q.w, t.q.x, t.q.y, t.q.z);
	glm::vec3 position(t.p.x, t.p.y, t.p.z);

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
	transform *= glm::mat4_cast(rotation);
	return transform;
}

