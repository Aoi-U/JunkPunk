#include "Vehicle.h"

Vehicle::Vehicle()
{
}

void Vehicle::InitPhysX()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = gGravity;

	PxU32 numWorkers = 1;
	gDispatcher = PxDefaultCpuDispatcherCreate(numWorkers);
	sceneDesc.cpuDispatcher = gDispatcher;
	//sceneDesc.filterShader = VehicleFilterShader;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	gScene = gPhysics->createScene(sceneDesc);
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(1.0f, 1.0f, 1.0f);

	PxInitVehicleExtension(*gFoundation);
}

void Vehicle::InitGroundPlane()
{
	gGroundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	for (PxU32 i = 0; i < gGroundPlane->getNbShapes(); i++)
	{
		PxShape* shape = NULL;
		gGroundPlane->getShapes(&shape, 1, i);
		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
	}
	gScene->addActor(*gGroundPlane);
}

void Vehicle::InitMaterialFrictionTable()
{
	//Each physx material can be mapped to a tire friction value on a per tire basis.
	//If a material is encountered that is not mapped to a friction value, the friction value used is the specified default value.
	//In this snippet there is only a single material so there can only be a single mapping between material and friction.
	//In this snippet the same mapping is used by all tires.
	gPhysXMaterialFrictions[0].friction = 1.0f;
	gPhysXMaterialFrictions[0].material = gMaterial;
	gPhysXDefaultMaterialFriction = 1.0f;
	gNbPhysXMaterialFrictions = 1;
}

bool Vehicle::InitVehicles()
{
	snippetvehicle::setPhysXIntegrationParams(gVehicle.mBaseParams.axleDescription,
		gPhysXMaterialFrictions,
		gNbPhysXMaterialFrictions,
		gPhysXDefaultMaterialFriction,
		gVehicle.mPhysXParams);

	if (!gVehicle.initialize(*gPhysics, PxCookingParams(PxTolerancesScale()), *gMaterial,
		snippetvehicle::EngineDriveVehicle::eDIFFTYPE_TANKDRIVE))
	{
		return false;
	}

	//Apply a start pose to the physx actor and add it to the physx scene.
	PxTransform pose(PxVec3(0.000000000f, -0.0500000119f, -1.59399998f), PxQuat(PxIdentity));
	gVehicle.setUpActor(*gScene, pose, gVehicleName);

	//Set the vehicle in 1st gear.
	gVehicle.mEngineDriveState.gearboxState.currentGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	gVehicle.mEngineDriveState.gearboxState.targetGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;

	//Set the vehicle to use automatic gears.
	gVehicle.mTankDriveTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

	//Set up the simulation context.
	//The snippet is set up with
	//a) z as the longitudinal axis
	//b) x as the lateral axis
	//c) y as the vertical axis.
	//d) metres  as the lengthscale.
	gVehicleSimulationContext.setToDefault();
	gVehicleSimulationContext.frame.lngAxis = PxVehicleAxes::ePosZ;
	gVehicleSimulationContext.frame.latAxis = PxVehicleAxes::ePosX;
	gVehicleSimulationContext.frame.vrtAxis = PxVehicleAxes::ePosY;
	gVehicleSimulationContext.scale.scale = 1.0f;
	gVehicleSimulationContext.gravity = gGravity;
	gVehicleSimulationContext.physxScene = gScene;
	gVehicleSimulationContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;
	return true;
}

bool Vehicle::InitPhysics()
{
	InitPhysX();
	InitGroundPlane();
	InitMaterialFrictionTable();
	if (!InitVehicles())
		return false;
	return true;
}

void Vehicle::stepPhysics()
{
	const PxReal timestep = 1.0f / 60.0f;	

	//Forward integrate the vehicle by a single timestep.
	//Apply substepping at low forward speed to improve simulation fidelity.
	const PxVec3 linVel = gVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity();
	const PxVec3 forwardDir = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector2();
	const PxReal forwardSpeed = linVel.dot(forwardDir);
	const PxU8 nbSubsteps = (forwardSpeed < 5.0f ? 3 : 1);
	gVehicle.mComponentSequence.setSubsteps(gVehicle.mComponentSequenceSubstepGroupHandle, nbSubsteps);
	gVehicle.step(timestep, gVehicleSimulationContext);

	//Forward integrate the phsyx scene by a single timestep.
	gScene->simulate(timestep);
	gScene->fetchResults(true);
}

void Vehicle::Simulate(float deltaTime)
{
	gScene->simulate(1/60.0f);
	gScene->fetchResults(true);
}

void Vehicle::Cleanup()
{
	gVehicle.destroy();
	gGroundPlane->release();
	cleanupPhysX();
}

void Vehicle::cleanupPhysX()
{
	PxCloseVehicleExtension();

	PX_RELEASE(gMaterial);
	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PX_RELEASE(gPhysics);
	if (gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		PX_RELEASE(gPvd);
		PX_RELEASE(transport);
	}
	PX_RELEASE(gFoundation);
}
