#include "MainVehicle.h"

MainVehicle::MainVehicle()
{
}

bool MainVehicle::setup(PxScene* scene, PxPhysics* physics, PxMaterial* material)
{
	initMaterialFrictionTable(material);

	if (!initVehicles(scene, physics, material))
	{
		std::cout << "Failed to setup vehicle!" << std::endl;
		return false;
	}
	return true;
}

void MainVehicle::initMaterialFrictionTable(PxMaterial* gMaterial)
{
	gPhysXMaterialFrictions[0].friction = 2.0f;
	gPhysXMaterialFrictions[0].material = gMaterial;
	gPhysXDefaultMaterialFriction = 1.5f;
	gNbPhysXmaterialFrictions = 1;
}

bool MainVehicle::initVehicles(PxScene* gScene, PxPhysics* gPhysics, PxMaterial* gMaterial)
{
	// Initialize the vehicle
	readBaseParamsFromJsonFile("assets/vehicledata/", "Base.json", gVehicle.mBaseParams);
	setPhysXIntegrationParams(gVehicle.mBaseParams.axleDescription, gPhysXMaterialFrictions, gNbPhysXmaterialFrictions, gPhysXDefaultMaterialFriction, gVehicle.mPhysXParams);
	readEngineDrivetrainParamsFromJsonFile("assets/vehicledata/", "EngineDrive.json", gVehicle.mEngineDriveParams);
	//	good params to tune for exaggerated movement:
	//		enginedrive.json:
	//			autoboxparams latency: lower values makes gearbox react faster to rpm changes
	//			engineparams torqueCurve: give full torque at lower rpm gives more power
	//			engineparams peaktorque: gives a good instant acceleration feeling
	//			engineparams maxOmega: higher values let engine rev higher which gets higher top speeds
	//			gearboxparams switchtime: lower values make gear shift faster
	//		base.json:
	//			rigidbodyparams mass: lower values make it accelerate easier and faster
	//			rigidbodyparams moi: lower values make turning very snappy and feels good, should focus more on this
	//			suspensionparams stiffness: higher values might reduce rolling over when turning
	//			suspensionparams damper: lower values make suspension bouncier
	// just experiment with everything i guess



	if (!gVehicle.initialize(*gPhysics, PxCookingParams(PxTolerancesScale()), *gMaterial, EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE))
	{
		std::cout << "Failed to initialize vehicle!" << std::endl;
		return false;
	}

	// Apply a start pose to the physx actor and add it to the physx scene
	PxTransform pose(PxVec3(0.0f, 10.0f, 0.0f), PxQuat(PxIdentity));
	// rotate around y 180 degrees
	PxQuat rot = PxQuat(PxPi, PxVec3(0.0f, 1.0f, 0.0f));
	pose.q = rot;
	gVehicle.setUpActor(*gScene, pose, gVehicleName);

	gVehicle.mEngineDriveState.gearboxState.currentGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	gVehicle.mEngineDriveState.gearboxState.targetGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;

	gVehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

	//PxShape* chassisShape = gPhysics->createShape(PxBoxGeometry(vph.physxActorBoxShapeHalfExtents), *gMaterial);
	PxShape* chassisShape = gPhysics->createShape(PxBoxGeometry(gVehicle.mPhysXParams.physxActorBoxShapeHalfExtents), *gMaterial);
	chassisShape->setFlag(PxShapeFlag::eVISUALIZATION, true);
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
	gVehicle.mPhysXState.physxActor.wheelShapes[0]->setFlag(PxShapeFlag::eVISUALIZATION, true);
	gVehicle.mPhysXState.physxActor.wheelShapes[1]->setFlag(PxShapeFlag::eVISUALIZATION, true);
	gVehicle.mPhysXState.physxActor.wheelShapes[2]->setFlag(PxShapeFlag::eVISUALIZATION, true);
	gVehicle.mPhysXState.physxActor.wheelShapes[3]->setFlag(PxShapeFlag::eVISUALIZATION, true);

	
	return true;
}

void MainVehicle::step(float deltaTime)
{
	//Apply the brake, throttle and steer to the command state of the direct drive vehicle.
	gVehicle.mCommandState.brakes[0] = gCommand.brake;
	gVehicle.mCommandState.nbBrakes = 1;
	gVehicle.mCommandState.throttle = gCommand.throttle;
	gVehicle.mCommandState.steer = gCommand.steer;

	//Forward integrate the vehicle by a single timestep.
	gVehicle.step(deltaTime, gVehicleSimulationContext);
}

void MainVehicle::cleanup()
{
	gVehicle.destroy();
}

void MainVehicle::setCommand(Command commands)
{
	gCommand = commands;
}

const PxTransform MainVehicle::getTransform() const
{
	PxTransform t = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();
	return t;
}

void MainVehicle::resetTransform()
{
	PxTransform t = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();

	glm::vec3 position(t.p.x, t.p.y + 1.0f, t.p.z); 
	glm::vec3 rotation(0.0f, 0.0f, 0.0f);

	setTransform(position, rotation);
}

void MainVehicle::setTransform(const glm::vec3& position, const glm::vec3& rotation)
{
	glm::quat rotQuat = glm::quat(rotation);
	PxQuat pxQuat(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w);
	PxVec3 pxPosition(position.x, position.y, position.z);
	PxTransform transform(pxPosition, pxQuat);

	gVehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(transform);
}

void MainVehicle::setCheckpoint(const glm::vec3& position, const glm::vec3& rotation)
{
	checkpointPosition = position;
	checkpointRotation = rotation;
}

void MainVehicle::respawnAtCheckpoint()
{
	setTransform(checkpointPosition, checkpointRotation);
}

const PxVec3 MainVehicle::getLinearVelocity() const
{
	return gVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity();
}

const PxVec3 MainVehicle::getAngularVelocity() const
{
	return gVehicle.mPhysXState.physxActor.rigidBody->getAngularVelocity();
}

bool MainVehicle::IsGrounded(PxScene* scene) const
{
	PxRaycastBuffer hit;
	bool grounded = false;
	PxVec3 rayOrigin = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	rayOrigin.y -= gVehicle.mPhysXParams.physxActorBoxShapeHalfExtents.y - 2.0f;
	PxVec3 rayDir = PxVec3(0.0f, -1.0f, 0.0f);
	PxReal rayLength = 1.0f;
	
	/*grounded = scene->raycast(rayOrigin, rayDir, rayLength, hit, PxHitFlag::eDEFAULT);
	std::cout << "Raycast hit count: " << hit.getNbAnyHits() << std::endl;
	std::cout << "Raycast hit object rigid body type: " << (hit.hasBlock ? hit.block.actor->getType() : -1) << std::endl;
	if (grounded)
	{
		return true;
	}*/
	return true;
}

void MainVehicle::jump()
{
	gVehicle.mPhysXState.physxActor.rigidBody->addForce(jumpForce, PxForceMode::eIMPULSE);
}

