#include "Vehicle.h"
//struct Cmd
//{
//	PxF32 brake;
//	PxF32 throttle;
//	PxF32 steer;
//	PxF32 duration;
//};
//const PxU32 gTargetGearCommand = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;
//Cmd gCommands[5] =
//{
//	{0.5f, 0.0f, 0.0f, 2.0f},		//brake on and come to rest for 2 seconds
//		{0.0f, 0.5f, 0.0f, 5.0f},		//throttle for 5 seconds
//		{0.5f, 0.0f, 0.0f, 5.0f},		//brake for 5 seconds
//	{0.0f, 0.5f, 0.0f, 5.0f},		//throttle for 5 seconds
//		{0.0f, 0.1f, 0.5f, 5.0f}		//light throttle and steer for 5 seconds.
//};
//const PxU32 gNbCommands = sizeof(gCommands) / sizeof(Cmd);
//PxReal gCommandTime = 0.0f;			//Time spent on current command
//PxU32 gCommandProgress = 0;			//The id of the current command.

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
	PxTransform pose(PxVec3(-5.0f, 0.5f, 0.0f), PxQuat(PxIdentity));
	gVehicle.setUpActor(*gScene, pose, gVehicleName);

	gVehicleSimulationContext.setToDefault();
	gVehicleSimulationContext.frame.lngAxis = PxVehicleAxes::ePosZ;
	gVehicleSimulationContext.frame.latAxis = PxVehicleAxes::ePosX;
	gVehicleSimulationContext.frame.vrtAxis = PxVehicleAxes::ePosY;
	gVehicleSimulationContext.scale.scale = 1.0f;
	gVehicleSimulationContext.gravity = gScene->getGravity();
	gVehicleSimulationContext.physxScene = gScene;
	gVehicleSimulationContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;

	return true;
}

void Vehicle::step(float deltaTime)
{
	/*std::cout << "Executing command: brake=" << gCommand.brake << " throttle=" << gCommand.throttle << " steer=" << gCommand.steer << std::endl;
	gVehicle.mCommandState.brakes[0] = gCommand.brake;
	gVehicle.mCommandState.throttle = gCommand.throttle;
	gVehicle.mCommandState.steer = gCommand.steer;

	gVehicle.step(1 / 60.0f, gVehicleSimulationContext);*/

	const PxF32 timestep = 0.0166667f;

	//Apply the brake, throttle and steer to the command state of the direct drive vehicle.
	gVehicle.mCommandState.brakes[0] = gCommand.brake;
	gVehicle.mCommandState.nbBrakes = 1;
	gVehicle.mCommandState.throttle = gCommand.throttle;
	gVehicle.mCommandState.steer = gCommand.steer;

	std::cout << "Initiating commands: brake=" << gVehicle.mCommandState.brakes[0] << " throttle=" << gVehicle.mCommandState.throttle << " steer=" << gVehicle.mCommandState.steer << std::endl;
	//Forward integrate the vehicle by a single timestep.
	gVehicle.step(timestep, gVehicleSimulationContext);

	//Forward integrate the phsyx scene by a single timestep.
	/*gScene->simulate(timestep);
	gScene->fetchResults(true);*/


}

void Vehicle::cleanup()
{
	gVehicle.destroy();
}

void Vehicle::setCommand(Command commands)
{
	gCommand = commands;
}

