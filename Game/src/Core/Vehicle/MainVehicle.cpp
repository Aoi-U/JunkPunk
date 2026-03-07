#include "MainVehicle.h"

MainVehicle::MainVehicle()
{
}

bool MainVehicle::setup(PxScene* scene, PxPhysics* physics, PxMaterial* material, PxTransform initTransform)
{
	initMaterialFrictionTable(material);

	if (!initVehicles(scene, physics, material, initTransform))
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

bool MainVehicle::initVehicles(PxScene* gScene, PxPhysics* gPhysics, PxMaterial* gMaterial, PxTransform initTransform)
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
	PxTransform pose(PxVec3(0.0f, -30.0f, 0.0f), PxQuat(PxIdentity));
	// rotate around y 180 degrees
	PxQuat rot = PxQuat(PxPi, PxVec3(0.0f, 1.0f, 0.0f));
	pose.q = rot;
	gVehicle.setUpActor(*gScene, initTransform, gVehicleName); // Might replace pose with initTransform

	PxFilterData vehicleFilter(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);
	PxFilterData wheelFilter(COLLISION_FLAG_WHEEL, COLLISION_FLAG_WHEEL_AGAINST, 0, 0);
	PxU32 shapes = gVehicle.mPhysXState.physxActor.rigidBody->getNbShapes();
	for (PxU32 i = 0; i < shapes; i++)
	{
		PxShape* shape = NULL;
		gVehicle.mPhysXState.physxActor.rigidBody->getShapes(&shape, 1, i);

		if (i == 0)
		{
			shape->setSimulationFilterData(vehicleFilter);
			shape->setQueryFilterData(vehicleFilter);

			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
		}
		else
		{
			shape->setSimulationFilterData(wheelFilter);
			shape->setQueryFilterData(wheelFilter);

			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
		}
	}

	gVehicle.mEngineDriveState.gearboxState.currentGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	gVehicle.mEngineDriveState.gearboxState.targetGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;

	gVehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

	//PxShape* chassisShape = gPhysics->createShape(PxBoxGeometry(vph.physxActorBoxShapeHalfExtents), *gMaterial);
	PxShape* chassisShape = gPhysics->createShape(PxBoxGeometry(gVehicle.mPhysXParams.physxActorBoxShapeHalfExtents), *gMaterial);
	chassisShape->setFlag(PxShapeFlag::eVISUALIZATION, true);

	PxFilterData chassisFilter(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);
	chassisShape->setSimulationFilterData(chassisFilter);
	chassisShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	chassisShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	chassisShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);

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
	gVehicle.mCommandState.nbBrakes = 1;
	gVehicle.mCommandState.steer = gCommand.steer;

	if (gCommand.throttle > 0.0f)
	{
		gVehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;
		gVehicle.mCommandState.throttle = gCommand.throttle;
	}
	else if (gCommand.brake > 0.0f)
	{
		gVehicle.mCommandState.steer = gCommand.steer * 0.5f; // reduce steer sensitivity when reversing
		gVehicle.mTransmissionCommandState.targetGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear - 1;
		gVehicle.mCommandState.throttle = gCommand.brake;
	}
	else
	{
		gVehicle.mCommandState.throttle = 0.0f;
	}

	//Forward integrate the vehicle by a single timestep.
	gVehicle.step(deltaTime, gVehicleSimulationContext);

	// stabalize vehicle when in the air to reduce vehicle flipping
	auto* dyn = gVehicle.mPhysXState.physxActor.rigidBody->is<PxRigidDynamic>();
	if (dyn && !IsGrounded(gVehicleSimulationContext.physxScene))
	{
		const float dampRate = 1.0f;
		const float dampFactor = expf(-dampRate * deltaTime);
		PxVec3 angVel = dyn->getAngularVelocity();
		angVel.x *= dampFactor;
		angVel.z *= dampFactor;
		dyn->setAngularVelocity(angVel);

		// add torque to vehicle to align it with the up direction
		PxTransform t = dyn->getGlobalPose();
		PxVec3 vehicleUp = t.q.rotate(PxVec3(0.0f, 1.0f, 0.0f));
		PxVec3 correctionTorque = vehicleUp.cross(PxVec3(0.0f, 1.0f, 0.0f)) * 10.0f;
		dyn->addTorque(correctionTorque, PxForceMode::eACCELERATION);
	}
}

void MainVehicle::setEntityUserData(Entity entity)
{
	gVehicle.mPhysXState.physxActor.rigidBody->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(entity));
}

void MainVehicle::cleanup()
{
	gVehicle.destroy();
}

void MainVehicle::setCommand(Command commands)
{
	gCommand = commands;
}

PxTransform MainVehicle::getWheelTransform(int wheelIndex) const
{
	if (wheelIndex < 0 || wheelIndex >= 4)
	{
		std::cerr << "Invalid wheel index: " << wheelIndex << std::endl;
		return PxTransform(PxVec3(0.0f), PxQuat(PxIdentity));
	}
	PxTransform vehicleWorldPose = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();
	PxTransform wheelLocalPose = gVehicle.mPhysXState.physxActor.wheelShapes[wheelIndex]->getLocalPose();
	PxTransform wheelWorldPose = vehicleWorldPose.transform(wheelLocalPose);

	return wheelWorldPose;
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
	glm::quat rotation(1.0, 0.0, 0.0, 0.0);

	setTransform(position, rotation);
}

void MainVehicle::setTransform(const glm::vec3& position, const glm::quat& rotation)
{
	PxQuat pxQuat(rotation.x, rotation.y, rotation.z, rotation.w);
	PxVec3 pxPosition(position.x, position.y, position.z);
	PxTransform transform(pxPosition, pxQuat);

	gVehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(transform);
}

void MainVehicle::setCheckpoint(const glm::vec3& position, const glm::quat& rotation)
{
	checkpointPosition = position;
	checkpointRotation = rotation;
	std::cout << "checkpoint reached at: " << checkpointPosition.x << " " << checkpointPosition.y << " " << checkpointPosition.z << std::endl;
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

bool MainVehicle::IsGrounded(const PxScene* scene) const
{
	PxRaycastBuffer hit;
	bool grounded = false;
	PxVec3 rayOrigin = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	rayOrigin.y -= (gVehicle.mPhysXParams.physxActorBoxShapeHalfExtents.y + 0.05f);
	PxVec3 rayDir = PxVec3(0.0f, -1.0f, 0.0f);
	PxReal rayLength = 0.3f;

	grounded = scene->raycast(rayOrigin, rayDir, rayLength, hit, PxHitFlag::eDEFAULT);
	if (grounded && hit.hasBlock && hit.block.actor != gVehicle.mPhysXState.physxActor.rigidBody)
	{
		return true;
	}

	return false;
}

void MainVehicle::jump()
{
	//gVehicle.mPhysXState.physxActor.rigidBody->addForce(jumpForce, PxForceMode::eIMPULSE);
	//PxRigidBodyExt::addForceAtLocalPos(*gVehicle.mPhysXState.physxActor.rigidBody, jumpForce, PxVec3(0.0f, -0.5f, 0.0f), PxForceMode::eIMPULSE);
	float jumpSpeed = sqrt(2.0 * 9.81 * 10.0);
	auto* body = gVehicle.mPhysXState.physxActor.rigidBody;
	auto dyn = body->is<PxRigidDynamic>();

	if (!dyn)
		return;

	PxVec3 velocity = dyn->getLinearVelocity();
	velocity.y = PxMax(velocity.y, 0.0f) + jumpSpeed;
	dyn->setLinearVelocity(velocity);
}

void MainVehicle::ApplyBoost(float multiplier) {
	auto& engine = gVehicle.mEngineDriveParams.engineParams;
	if (basePeakTorque < 0.0f)
		basePeakTorque = engine.peakTorque;

	engine.peakTorque = basePeakTorque * multiplier;
}

void MainVehicle::ClearBoost() {
	if (basePeakTorque < 0.0f)
		return;
	gVehicle.mEngineDriveParams.engineParams.peakTorque = basePeakTorque;
}

void MainVehicle::SpinOut()
{
	auto* body = gVehicle.mPhysXState.physxActor.rigidBody;
	if (!body) return;

	body->addTorque(PxVec3(0.0f, 800.0f, 0.0f), PxForceMode::eIMPULSE);
}
